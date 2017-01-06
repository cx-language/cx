cmake_minimum_required(VERSION 3.1)
project(delta)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

enable_testing()

add_subdirectory(src/ast)
add_subdirectory(src/parser)
add_subdirectory(test/parser)

# Add 'check' target with --output-on-failure.
if (CMAKE_CONFIGURATION_TYPES)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
        --force-new-ctest-process --output-on-failure
        --build-config "$<CONFIGURATION>")
else()
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
        --force-new-ctest-process --output-on-failure)
endif()
