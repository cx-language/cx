add_test(NAME typecheck-test
    COMMAND typecheck-tester.sh $<TARGET_FILE:delta>
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
