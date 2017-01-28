add_test(NAME irgen-test-validate-ir
    COMMAND validate-ir.sh $<TARGET_FILE:delta>
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
