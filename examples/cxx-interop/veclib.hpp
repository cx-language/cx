#pragma once
#include <vector>

struct Accum {
    int count;
    int total;
};

int vec_sum(const std::vector<int>& values);
double vec_mean(const std::vector<int>& values);
int accum_add(Accum accum, int x);
