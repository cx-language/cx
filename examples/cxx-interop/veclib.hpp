#pragma once

struct Accum {
    int count;
    int total;
};

int accum_add(Accum accum, int x);
