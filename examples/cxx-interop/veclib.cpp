#include "veclib.hpp"

int vec_sum(const std::vector<int>& values) {
    int sum = 0;
    for (int x : values) sum += x;
    return sum;
}

double vec_mean(const std::vector<int>& values) {
    if (values.empty()) return 0.0;
    return double(vec_sum(values)) / double(values.size());
}

int accum_add(Accum accum, int x) {
    return accum.total + x;
}
