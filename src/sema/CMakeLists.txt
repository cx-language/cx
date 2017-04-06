file(GLOB SOURCES *.h *.cpp)
add_library(deltaSema ${SOURCES})
target_link_libraries(deltaSema deltaAST
    clangAST clangBasic clangFrontend clangLex clangParse)
