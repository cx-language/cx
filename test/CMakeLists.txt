add_test(NAME lit-test
    COMMAND lit --verbose ${CMAKE_CURRENT_LIST_DIR}
    -Ddelta_path="$<TARGET_FILE:delta>"
    -Dfilecheck_path="$<TARGET_FILE:FileCheck>"
    -Dnot_path="$<TARGET_FILE:not>"
    -Dcheck_exit_status_path="${CMAKE_CURRENT_LIST_DIR}/check_exit_status.sh")
