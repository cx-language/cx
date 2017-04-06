file(GLOB SOURCES *.h *.cpp)
add_library(deltaAST ${SOURCES})
target_include_directories(deltaAST SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
llvm_map_components_to_libnames(LLVM_LIBS support) # for raw_ostream
target_link_libraries(deltaAST ${LLVM_LIBS} ${Boost_LIBRARIES})
