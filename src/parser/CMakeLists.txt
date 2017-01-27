find_package(BISON REQUIRED)

BISON_TARGET(DeltaParser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp)

add_library(deltaParser lexer.cpp ${BISON_DeltaParser_OUTPUTS})
target_link_libraries(deltaParser deltaAST deltaSema)
