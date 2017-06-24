file(GLOB SOURCES *.h *.cpp)
add_library(deltaAST ${SOURCES})
llvm_map_components_to_libnames(LLVM_LIBS support) # for raw_ostream
target_link_libraries(deltaAST ${LLVM_LIBS})
