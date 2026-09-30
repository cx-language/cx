// Word frequency counter, matching wordcount.cx: xorshift64 word stream,
// per-word allocations, growing open-addressing table, order-independent
// aggregate. Keys are views into the owning word list, like Map<string, int>.
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
static size_t tableCap;
static size_t tableSize;

static void tableReinsert(const char* key, size_t keyLen, int count) {
    size_t mask = tableCap - 1;
    size_t i = fnv1a(key, keyLen) & mask;
    while (table[i].used)
        i = (i + 1) & mask;
    table[i].used = true;
    table[i].key = key;
    table[i].keyLen = keyLen;
    table[i].count = count;
}

static void tableGrow(void) {
    size_t oldCap = tableCap;
    Entry* old = table;
    tableCap *= 2;
    table = calloc(tableCap, sizeof(Entry));
    for (size_t i = 0; i < oldCap; i++) {
        if (old[i].used) tableReinsert(old[i].key, old[i].keyLen, old[i].count);
    }
    free(old);
}

static void tableAdd(const char* key, size_t keyLen) {
    if ((tableSize + 1) * 4 >= tableCap * 3) tableGrow();
    size_t mask = tableCap - 1;
    size_t i = fnv1a(key, keyLen) & mask;
    for (;;) {
        if (!table[i].used) {
            table[i].used = true;
            table[i].key = key;
            table[i].keyLen = keyLen;
            table[i].count = 1;
            tableSize++;
            return;
        }
        if (table[i].keyLen == keyLen && memcmp(table[i].key, key, keyLen) == 0) {
            table[i].count++;
            return;
        }
        i = (i + 1) & mask;
    }
}

int main(void) {
    const size_t syllableCount = sizeof(syllables) / sizeof(syllables[0]);
    char** words = malloc(4000000u * sizeof(char*));
    tableCap = 128;
    table = calloc(tableCap, sizeof(Entry));

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
        size_t lenA = strlen(syllables[a]);
        size_t lenB = strlen(syllables[b]);
        char* word = malloc(lenA + lenB + 1);
        memcpy(word, syllables[a], lenA);
        memcpy(word + lenA, syllables[b], lenB);
        word[lenA + lenB] = '\0';
        words[n] = word;
        tableAdd(word, lenA + lenB);
    }

    long long total = 0;
    long long sumSquares = 0;
    for (size_t i = 0; i < tableCap; i++) {
        if (!table[i].used) continue;
        total += table[i].count;
        sumSquares += (long long)table[i].count * table[i].count;
    }
    printf("%lld\n", (long long)tableSize * 1000000000000LL + total * 1000000LL + sumSquares);
    for (int n = 0; n < 4000000; n++)
        free(words[n]);
    free(words);
    free(table);
    return 0;
}
