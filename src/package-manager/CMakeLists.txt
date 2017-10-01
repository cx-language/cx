file(GLOB SOURCES *.h *.cpp)
add_library(deltaPackageManager ${SOURCES})
llvm_map_components_to_libnames(LLVM_LIBS support)
target_link_libraries(deltaPackageManager deltaSupport ${LLVM_LIBS})
