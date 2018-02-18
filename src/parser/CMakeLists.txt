file(GLOB SOURCES *.h *.cpp)
add_library(deltaParser ${SOURCES})
target_link_libraries(deltaParser deltaAST deltaSupport)
