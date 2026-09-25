// Calls cx from C. Build the cx side first (see README.md).

#include <stdio.h>
#include "cxlib.h"

int main(void) {
    printf("%d\n", cx_is_even(42)); // prints 1
    printf("%f\n", cx_hypot(3.0, 4.0)); // prints 5.000000
    return 0;
}
