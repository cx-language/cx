// Word frequency counter, matching bench/wordcount.cx: xorshift64 word stream,
// open-addressing string table, order-independent aggregate.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* syllables[] = {"al", "be", "cor", "di", "el", "for", "gi", "ha", "il", "jo", "ka",  "li",  "ma",  "no",  "or",  "pa",
                                  "qu", "ra", "si",  "ta", "ul", "vi",  "wo", "xa", "ya", "zo", "ash", "bel", "cam", "dor", "esh", "fim"};

static uint64_t fnv1a(const char* s, size_t len) {
    uint64_t hash = 1469598103934665603u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t)(unsigned char)s[i];
        hash *= 1099511628211u;
    }
    return hash;
}

typedef struct {
    const char* key;
    size_t keyLen;
    int count;
    bool used;
} Entry;

static Entry* table;
static size_t tableMask;
// At most 32*32 distinct words ever land in the 4096-entry table, so the
// insert loop below always terminates; no full-table guard needed.

static void tableInsert(const char* key, size_t keyLen) {
    size_t i = fnv1a(key, keyLen) & tableMask;
    while (table[i].used) {
        if (table[i].keyLen == keyLen && memcmp(table[i].key, key, keyLen) == 0) {
            table[i].count++;
            return;
        }
        i = (i + 1) & tableMask;
    }
    table[i].used = true;
    table[i].key = key;
    table[i].keyLen = keyLen;
    table[i].count = 1;
}

int main(void) {
    const size_t syllableCount = sizeof(syllables) / sizeof(syllables[0]);
    // 4M words of at most 6 chars plus NUL, pooled like the cx List<StringBuf>.
    char* pool = malloc(4000000u * 7u);
    char* poolNext = pool;
    table = calloc(4096, sizeof(Entry));
    tableMask = 4095;

    uint64_t state = 0x12345678u;
    for (int n = 0; n < 4000000; n++) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        size_t a = (size_t)(state % syllableCount);
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        size_t b = (size_t)(state % syllableCount);
        char* word = poolNext;
        size_t len = 0;
        for (const char* s = syllables[a]; *s; s++)
            word[len++] = *s;
        for (const char* s = syllables[b]; *s; s++)
            word[len++] = *s;
        word[len] = '\0';
        poolNext += len + 1;
        tableInsert(word, len);
    }

    long long total = 0;
    long long sumSquares = 0;
    long long distinct = 0;
    for (size_t i = 0; i < 4096; i++) {
        if (!table[i].used) continue;
        distinct++;
        total += table[i].count;
        sumSquares += (long long)table[i].count * table[i].count;
    }
    printf("%lld\n", distinct * 1000000000000LL + total * 1000000LL + sumSquares);
    free(pool);
    free(table);
    return 0;
}
