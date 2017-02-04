file(GLOB SOURCES *.h *.cpp)
add_library(deltaIRGen ${SOURCES})
target_link_libraries(deltaIRGen deltaAST)

llvm_map_components_to_libnames(LLVM_LIBS core native)
target_link_libraries(deltaIRGen ${LLVM_LIBS}) # Link against LLVM libraries
