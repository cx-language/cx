// Filter/map pipeline plus capturing closure, matching mapfilter.cx.
#include <cstdio>
#include <ranges>
#include <vector>

namespace {

const int divisor = 7;
const int factor = 3;

} // namespace

int main() {
    std::vector<int> numbers;
    numbers.reserve(4000000);
    for (int i = 0; i < 4000000; i++)
        numbers.push_back(i);
    long long total = 0;
    for (int round = 0; round < 40; round++) {
        for (int x : numbers | std::views::filter([](int n) { return n % divisor == 0; }) | std::views::transform([](int n) { return n * factor + 1; })
                         | std::views::filter([](int n) { return n % 2 == 0; })) {
            total += x;
        }
    }
    const int offset = 1;
    auto adjust = [](int n) { return n * factor + offset; };
    for (int i = 0; i < 8000000; i++)
        total += adjust(i);
    std::printf("%lld\n", total);
}
