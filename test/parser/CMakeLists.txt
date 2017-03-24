add_test(NAME parser-test
    COMMAND parser-tester.sh $<TARGET_FILE:delta>
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
