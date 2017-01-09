file(GLOB SOURCES *.h *.cpp)
add_library(deltaCGen ${SOURCES})
target_include_directories(deltaCGen SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(deltaCGen deltaAST ${Boost_LIBRARIES})
