// Word frequency counter, matching wordcount.cx: xorshift64 word stream,
// hash map inserts and lookups, order-independent aggregate.
#include <cstdint>
#include <cstdio>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

const char* syllables[] = {"al", "be", "cor", "di", "el", "for", "gi", "ha", "il", "jo", "ka",  "li",  "ma",  "no",  "or",  "pa",
                           "qu", "ra", "si",  "ta", "ul", "vi",  "wo", "xa", "ya", "zo", "ash", "bel", "cam", "dor", "esh", "fim"};

} // namespace

int main() {
    const size_t syllableCount = sizeof(syllables) / sizeof(syllables[0]);
    std::vector<std::string> words;
    words.reserve(4000000);
    std::unordered_map<std::string, int> counts;
    uint64_t state = 0x12345678u;
    for (int n = 0; n < 4000000; n++) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        size_t a = state % syllableCount;
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        size_t b = state % syllableCount;
        words.emplace_back(syllables[a]);
        words.back() += syllables[b];
        counts[words.back()]++;
    }
    long long total = 0;
    long long sumSquares = 0;
    for (int count : counts | std::views::values) {
        total += count;
        sumSquares += static_cast<long long>(count) * count;
    }
    std::printf("%lld\n", static_cast<long long>(counts.size()) * 1000000000000LL + total * 1000000LL + sumSquares);
}
