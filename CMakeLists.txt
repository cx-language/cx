cmake_minimum_required(VERSION 3.1)
project(delta)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-fno-rtti -fvisibility=hidden ${CMAKE_CXX_FLAGS}")

find_package(LLVM 3.9 REQUIRED)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

find_package(Clang REQUIRED)

find_package(Boost REQUIRED)

enable_testing()

add_subdirectory(src/ast)
add_subdirectory(src/cgen)
add_subdirectory(src/driver)
add_subdirectory(src/irgen)
add_subdirectory(src/parser)
add_subdirectory(src/sema)
add_subdirectory(test/cgen)
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
