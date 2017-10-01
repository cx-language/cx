file(GLOB SOURCES *.h *.cpp)
add_executable(delta ${SOURCES})
target_link_libraries(delta deltaDriver)
