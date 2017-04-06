file(GLOB SOURCES *.h *.cpp)
add_library(deltaIRGen ${SOURCES})
llvm_map_components_to_libnames(LLVM_LIBS core)
target_link_libraries(deltaIRGen deltaAST deltaSema ${LLVM_LIBS})
