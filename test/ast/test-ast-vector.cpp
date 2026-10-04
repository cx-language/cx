#include "ast/arena.h"
#include <cassert>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#pragma warning(pop)

namespace {

int destructions = 0;

struct Counted {
    int value;
    Counted(int value = 0) : value(value) {}
    ~Counted() { ++destructions; }
};

} // namespace

int main() {
    using cx::AstVector;

    // All vectors die before the reset below: their destructors must not run
    // over freed slabs.
    {
    // Functional smoke: full std::vector API over arena buffers.
    AstVector<int> grown;
    for (int i = 0; i < 100; ++i)
        grown.push_back(i);
    assert(grown.size() == 100 && grown.front() == 0 && grown.back() == 99);
    grown.insert(grown.begin() + 1, 2, -1);
    assert(grown.size() == 102 && grown[1] == -1);
    grown.erase(grown.begin(), grown.begin() + 2);
    assert(grown.size() == 100 && grown[0] == -1);
    grown.resize(3);
    assert((grown == AstVector<int>{-1, 1, 2}));

    AstVector<int> copied = grown;
    assert(copied == grown && copied.data() != grown.data());
    const int* beforeMove = copied.data();
    AstVector<int> moved = std::move(copied);
    assert(moved == grown && moved.data() == beforeMove && copied.empty());

    llvm::ArrayRef<int> ref = grown;
    assert(ref.size() == 3 && ref[0] == -1 && ref[2] == 2);
    llvm::MutableArrayRef<int> mut = grown;
    mut[0] = 10;
    assert(grown[0] == 10);

    // The destructor destroys elements; buffers stay in the arena.
    destructions = 0;
    {
        AstVector<Counted> objects;
        objects.reserve(2);
        objects.emplace_back(1);
        objects.emplace_back(2);
    }
    assert(destructions == 2);

    // Growth abandons (never frees) the old buffer: it stays readable.
    AstVector<int> small;
    small.push_back(1);
    const int* firstBuffer = small.data();
    for (int i = 0; i < 100; ++i)
        small.push_back(i);
    assert(small.data() != firstBuffer && *firstBuffer == 1);
    }

    // The arena is reusable after a reset.
    cx::resetAstArena();
    AstVector<int> after = {1, 2};
    assert(after.size() == 2 && after[1] == 2);
}
