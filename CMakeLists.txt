cmake_minimum_required(VERSION 3.1)
project(delta)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-fno-rtti -fvisibility-inlines-hidden -Wall -Wextra ${CMAKE_CXX_FLAGS}")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "--system-header-prefix=llvm --system-header-prefix=clang ${CMAKE_CXX_FLAGS}")
endif()

find_package(LLVM 4.0 REQUIRED)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

find_package(Clang REQUIRED)

# Define the include path of the Clang builtin headers, to be used by the compiler to avoid errors
# about missing C headers when importing C headers from Delta code. For more information, see e.g.
# http://clang.llvm.org/docs/FAQ.html#i-get-errors-about-some-headers-being-missing-stddef-h-stdarg-h
# CLANG_INSTALL_PREFIX is defined by ClangConfig.cmake during the above find_package(Clang) call.
add_definitions(-DCLANG_BUILTIN_INCLUDE_PATH="${CLANG_INSTALL_PREFIX}/lib/clang/${LLVM_VERSION}/include")

find_package(PkgConfig)
pkg_check_modules(LIBEDIT REQUIRED libedit)

enable_testing()

add_subdirectory(src/ast)
add_subdirectory(src/driver)
add_subdirectory(src/irgen)
add_subdirectory(src/parser)
add_subdirectory(src/sema)
add_subdirectory(src/support)
add_subdirectory(test)
add_subdirectory(test/misc)
add_subdirectory(test/parser)
add_subdirectory(test/sema)

if(CMAKE_CONFIGURATION_TYPES)
    set(BUILD_CONFIG --build-config $<CONFIGURATION>)
endif()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
    --force-new-ctest-process --output-on-failure ${BUILD_CONFIG} USES_TERMINAL)

# Download the LLVM FileCheck utility for tests.
set(FILECHECK_SRC_URL "https://raw.githubusercontent.com/llvm-mirror/llvm/release_${LLVM_VERSION_MAJOR}${LLVM_VERSION_MINOR}/utils/FileCheck/FileCheck.cpp")
set(FILECHECK_SRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/FileCheck.cpp")
if(NOT EXISTS "${FILECHECK_SRC_FILE}")
    message(STATUS "Downloading LLVM FileCheck source")
    file(DOWNLOAD "${FILECHECK_SRC_URL}" "${FILECHECK_SRC_FILE}")
endif()
add_executable(FileCheck ${FILECHECK_SRC_FILE})
target_compile_options(FileCheck PRIVATE -w)
llvm_map_components_to_libnames(FILECHECK_NEEDED_LIBS support)
target_link_libraries(FileCheck PRIVATE ${FILECHECK_NEEDED_LIBS})

# Download the LLVM 'not' utility for tests.
set(NOT_SRC_URL "https://raw.githubusercontent.com/llvm-mirror/llvm/release_${LLVM_VERSION_MAJOR}${LLVM_VERSION_MINOR}/utils/not/not.cpp")
set(NOT_SRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/not.cpp")
if(NOT EXISTS "${NOT_SRC_FILE}")
    message(STATUS "Downloading LLVM 'not' source")
    file(DOWNLOAD "${NOT_SRC_URL}" "${NOT_SRC_FILE}")
endif()
add_executable(not ${NOT_SRC_FILE})
target_compile_options(not PRIVATE -w)
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
        setup_target_for_coverage(coverage ctest coverage)
    else()
        message(STATUS "lcov not found! Skipping 'coverage' target...")
    endif()
endif()

add_custom_target(format COMMAND uncrustify -c ${PROJECT_SOURCE_DIR}/.uncrustify.cfg --no-backup
    ${PROJECT_SOURCE_DIR}/src/**/*.h ${PROJECT_SOURCE_DIR}/src/**/*.cpp)
