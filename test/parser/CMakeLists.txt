file(GLOB TEST_FILES *.delta)
add_test(NAME parser-test COMMAND delta ${TEST_FILES})
