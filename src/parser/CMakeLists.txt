find_package(BISON REQUIRED)

BISON_TARGET(DeltaParser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp)

file(GLOB SOURCES *.h *.cpp)
add_library(deltaParser ${SOURCES} ${BISON_DeltaParser_OUTPUTS})
target_link_libraries(deltaParser deltaAST deltaSema)

# lexer.cpp and operators.cpp are compiled via parser.y
set_source_files_properties(lexer.cpp operators.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
