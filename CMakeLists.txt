cmake_minimum_required(VERSION 3.8)
project(delta)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "-w14062 ${CMAKE_CXX_FLAGS}")
else()
    set(CMAKE_CXX_FLAGS "-fno-rtti -fvisibility-inlines-hidden -Wall -Wextra -Wno-unknown-pragmas ${CMAKE_CXX_FLAGS}")
    if(NOT MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "-rdynamic ${CMAKE_EXE_LINKER_FLAGS}")
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "--system-header-prefix=llvm --system-header-prefix=clang ${CMAKE_CXX_FLAGS}")
endif()

find_package(LLVM 7.1)
if(NOT LLVM_FOUND)
    find_package(LLVM 7.0 REQUIRED)
endif()
find_package(Clang REQUIRED)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS} -DDELTA_ROOT_DIR="${PROJECT_SOURCE_DIR}")

# Define the include path of the Clang builtin headers, to be used by the compiler to avoid errors
# about missing C headers when importing C headers from Delta code. For more information, see e.g.
# http://clang.llvm.org/docs/FAQ.html#i-get-errors-about-some-headers-being-missing-stddef-h-stdarg-h
# CLANG_CMAKE_DIR is defined by ClangConfig.cmake during the above find_package(Clang) call.
add_definitions(-DCLANG_BUILTIN_INCLUDE_PATH="${CLANG_CMAKE_DIR}/../../clang/${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}/include")

find_package(PkgConfig)
pkg_check_modules(LIBEDIT libedit)

enable_testing()

file(GLOB LIBRARY_SOURCES src/ast/*.h src/ast/*.cpp src/driver/*.h src/driver/*.cpp src/irgen/*.h src/irgen/*.cpp src/package-manager/*.h
    src/package-manager/*.cpp src/parser/*.h src/parser/*.cpp src/sema/*.h src/sema/*.cpp src/support/*.h src/support/*.cpp)
file(GLOB DELTA_SOURCES src/delta/*.h src/delta/*.cpp)

llvm_map_components_to_libnames(LLVM_LIBS core native mc lineeditor executionengine interpreter bitwriter linker support)
list(APPEND LLVM_LIBS clangAST clangBasic clangFrontend clangLex clangParse clangSema)

if(BUILD_LIBDELTA)
    add_library(libdelta ${LIBRARY_SOURCES})
    add_executable(delta ${DELTA_SOURCES})
    target_link_libraries(libdelta ${LLVM_LIBS})
    target_link_libraries(delta libdelta)
    set_target_properties(libdelta PROPERTIES PREFIX "")
else()
    add_executable(delta ${LIBRARY_SOURCES} ${DELTA_SOURCES})
    target_link_libraries(delta ${LLVM_LIBS})
endif()

if(MSVC)
    # For some reason running the tests multi-threadedly on Windows causes random tests to fail occasionally.
    # TODO: Figure out what's going on.
    set(EXTRA_LIT_FLAGS "--threads=1")
endif()

if(DEFINED ENV{CI})
    # Work around occasional "error: couldn't import the standard library: No such file or directory" in CI.
    set(EXTRA_LIT_FLAGS "--threads=1")
endif()

add_custom_target(check_lit COMMAND lit --verbose --succinct --incremental ${EXTRA_LIT_FLAGS} ${PROJECT_SOURCE_DIR}/test
    -Ddelta_path="$<TARGET_FILE:delta>"
    -Dfilecheck_path="$<TARGET_FILE:FileCheck>"
    -Dnot_path="$<TARGET_FILE:not>"
    -Dtest_helper_scripts_path="${PROJECT_SOURCE_DIR}/test"
    USES_TERMINAL)
add_custom_target(check_examples COMMAND "${PROJECT_SOURCE_DIR}/examples/build_examples.sh" "$<TARGET_FILE:delta>")
add_custom_target(check)
add_custom_target(update_snapshots ${CMAKE_COMMAND} -E env UPDATE_SNAPSHOTS=1 cmake --build "${CMAKE_BINARY_DIR}" --target check)
add_dependencies(check check_lit check_examples)

if(MSVC)
    # If we try to set '-Ox' here, we get the error "'/Ox' and '/RTC1' command-line options are incompatible",
    # and there seems to be no easy way to override compile flags for a specific target for MSVC from CMake.
    set(FILECHECK_COMPILE_FLAGS "-w")
    set(NOT_COMPILE_FLAGS "-w")
else()
    set(FILECHECK_COMPILE_FLAGS "-w -O3")
    set(NOT_COMPILE_FLAGS "-w -O0") # Don't optimize 'not', otherwise it segfaults on CI (since LLVM 7).
endif()

# Download the LLVM FileCheck utility for tests.
set(FILECHECK_SRC_PATH "llvmorg-${LLVM_PACKAGE_VERSION}/llvm/utils/FileCheck/FileCheck.cpp")
set(FILECHECK_SRC_URL "https://raw.githubusercontent.com/llvm/llvm-project/${FILECHECK_SRC_PATH}")
set(FILECHECK_SRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILECHECK_SRC_PATH}")
if(NOT EXISTS "${FILECHECK_SRC_FILE}")
    message(STATUS "Downloading LLVM FileCheck source")
    file(DOWNLOAD "${FILECHECK_SRC_URL}" "${FILECHECK_SRC_FILE}")
endif()
add_executable(FileCheck ${FILECHECK_SRC_FILE})
set_target_properties(FileCheck PROPERTIES COMPILE_FLAGS "${LLVM_DEFINITIONS} ${FILECHECK_COMPILE_FLAGS}")
llvm_map_components_to_libnames(FILECHECK_NEEDED_LIBS support)
target_link_libraries(FileCheck PRIVATE ${FILECHECK_NEEDED_LIBS})

# Download the LLVM 'not' utility for tests.
set(NOT_SRC_PATH "llvmorg-${LLVM_PACKAGE_VERSION}/llvm/utils/not/not.cpp")
set(NOT_SRC_URL "https://raw.githubusercontent.com/llvm/llvm-project/${NOT_SRC_PATH}")
set(NOT_SRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NOT_SRC_PATH}")
if(NOT EXISTS "${NOT_SRC_FILE}")
    message(STATUS "Downloading LLVM 'not' source")
    file(DOWNLOAD "${NOT_SRC_URL}" "${NOT_SRC_FILE}")
endif()
add_executable(not ${NOT_SRC_FILE})
set_target_properties(not PROPERTIES COMPILE_FLAGS "${LLVM_DEFINITIONS} ${NOT_COMPILE_FLAGS}")
llvm_map_components_to_libnames(NOT_NEEDED_LIBS support)
target_link_libraries(not PRIVATE ${NOT_NEEDED_LIBS})

add_dependencies(check FileCheck not)

# Add 'coverage' target for code coverage report generation.
file(DOWNLOAD https://github.com/bilke/cmake-modules/raw/9dc1ac2dfb7ed7b0be80eec17127d857d702b2ed/CodeCoverage.cmake
    ${CMAKE_BINARY_DIR}/cmake-modules/CodeCoverage.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/cmake-modules)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    include(CodeCoverage)
    set(LCOV_REMOVE_EXTRA '*/v1/*')
    if(LCOV_PATH)
        setup_target_for_coverage(coverage "cmake --build ${CMAKE_BINARY_DIR} --target check" coverage)
    else()
        message(STATUS "lcov not found! Skipping 'coverage' target...")
    endif()
endif()

file(DOWNLOAD https://github.com/llvm/llvm-project/raw/master/clang-tools-extra/clang-tidy/tool/run-clang-tidy.py
    ${CMAKE_BINARY_DIR}/run-clang-tidy.py)
add_custom_target(lint COMMAND "${PROJECT_SOURCE_DIR}/scripts/format.sh" "${CMAKE_BINARY_DIR}" --check)
add_custom_target(format COMMAND "${PROJECT_SOURCE_DIR}/scripts/format.sh" "${CMAKE_BINARY_DIR}" --skip-clang-tidy)

if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
    if(EXISTS "${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit")
        file(COPY "${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit" DESTINATION "${PROJECT_SOURCE_DIR}/.git/hooks/backup")
        message(STATUS "Updating git pre-commit hook (old hook backed up into .git/hooks/backup)")
    else()
        message(STATUS "Installing git pre-commit hook")
    endif()
    configure_file("${PROJECT_SOURCE_DIR}/scripts/pre-commit" "${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit" @ONLY NEWLINE_STYLE LF)
endif()
