add_test(NAME misc-test
    COMMAND tester.sh $<TARGET_FILE:delta>
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
