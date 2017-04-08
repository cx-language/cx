cmake_minimum_required(VERSION 3.1)
project(delta)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-fno-rtti -fvisibility-inlines-hidden -Wall -Wextra ${CMAKE_CXX_FLAGS}")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "--system-header-prefix=llvm --system-header-prefix=clang ${CMAKE_CXX_FLAGS}")
endif()

find_package(LLVM 4.0)
if(NOT LLVM_FOUND)
    find_package(LLVM 3.9 REQUIRED)
endif()
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
# Use old libstdc++ ABI to avoid linker errors with prebuilt LLVM
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

find_package(Clang REQUIRED)

find_package(Boost REQUIRED)

enable_testing()

add_subdirectory(src/ast)
add_subdirectory(src/driver)
add_subdirectory(src/irgen)
add_subdirectory(src/parser)
add_subdirectory(src/sema)
add_subdirectory(test)
add_subdirectory(test/irgen)
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

# Add 'coverage' target for code coverage report generation.
file(DOWNLOAD https://github.com/bilke/cmake-modules/raw/master/CodeCoverage.cmake
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
