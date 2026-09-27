#pragma once

// A minimal self-contained std::vector stand-in (no system headers), shaped
// like the real one: a vector template with a defaulted allocator in
// namespace std, plus a custom allocator and a bool-typed use.
namespace std {
template<class T> struct allocator {};
template<class T, class A = allocator<T>> struct vector {
    T* begin_;
    T* end_;
    T* cap_;
};
template<class T> struct MyAlloc : allocator<T> {};
} // namespace std

struct Item {
    int weight;
};

struct FPoint {
    int x, y;
};

struct WithDtor {
    int x;
    ~WithDtor() {}
};

struct Big {
    int vals[8];
};

struct FPair {
    float x, y;
};

struct Sixteen {
    int v[4];
};

struct Flags {
    unsigned x : 3;
    unsigned y : 5;
};

int if_sum(const std::vector<int>& v);
int if_first(std::vector<int> v);
int if_boolsum(const std::vector<bool>& v);
int if_custom(const std::vector<int, std::MyAlloc<int>>& v);
int if_compound(const std::vector<Item*>& v);
int if_pdot(FPoint p);
int if_dtorval(WithDtor w);
FPoint if_mkpoint(int x);
int if_big(Big b);
int if_fpair(FPair p);
int if_sixteen(Sixteen s);
int if_flags(Flags f);
void if_bigcb(void (*cb)(Big b));
typedef void (*BigCb)(Big b);
BigCb if_getcb(void);
extern "C" int if_cdouble(int x);
