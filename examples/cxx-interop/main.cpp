#include <cstdio>
#include "cxlib.hpp"

int main() {
    printf("%d\n", cx_is_even(42));
    printf("%d\n", cx_is_even(7));
    std::vector<int> v = {10, 20, 30};
    printf("%d\n", cx_vec_sum(v));
    return 0;
}
