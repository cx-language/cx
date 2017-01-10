file(GLOB SOURCES *.h *.cpp)
add_library(deltaAST ${SOURCES})
target_include_directories(deltaAST SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(deltaAST deltaParser ${Boost_LIBRARIES})
