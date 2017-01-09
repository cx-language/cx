file(GLOB TEST_FILES *.delta)
add_test(NAME parser-test COMMAND delta -fsyntax-only ${TEST_FILES})
add_test(NAME simple-parser-test-validate-ast
    COMMAND validate-ast.sh $<TARGET_FILE:delta> simple-parser-test.delta
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
