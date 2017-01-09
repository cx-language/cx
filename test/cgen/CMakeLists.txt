add_test(NAME simple-cgen-test
    COMMAND compile.sh $<TARGET_FILE:delta> simple-cgen-test.delta
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
