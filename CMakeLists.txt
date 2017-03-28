cmake_minimum_required(VERSION 3.1)
project(delta)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-fno-rtti -fvisibility=hidden ${CMAKE_CXX_FLAGS}")

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
add_subdirectory(test/irgen)
add_subdirectory(test/misc)
add_subdirectory(test/parser)
add_subdirectory(test/sema)

# Add 'check' target with --output-on-failure.
if (CMAKE_CONFIGURATION_TYPES)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
        --force-new-ctest-process --output-on-failure
        --build-config "$<CONFIGURATION>" USES_TERMINAL)
else()
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
        --force-new-ctest-process --output-on-failure USES_TERMINAL)
endif()

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
