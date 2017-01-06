find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

FLEX_TARGET(DeltaLexer
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp)
BISON_TARGET(DeltaParser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp)
ADD_FLEX_BISON_DEPENDENCY(DeltaLexer DeltaParser)

add_executable(delta main.cpp
    ${FLEX_DeltaLexer_OUTPUTS} ${BISON_DeltaParser_OUTPUTS})
target_link_libraries(delta deltaAST)
