// Filter/map pipeline plus capturing closure, matching bench/mapfilter.cx.
#include <stdio.h>
#include <stdlib.h>

static const int divisor = 7;
static const int factor = 3;

int main(void) {
    int* numbers = malloc(4000000u * sizeof(int));
    for (int i = 0; i < 4000000; i++)
        numbers[i] = i;
    long long total = 0;
    for (int round = 0; round < 40; round++) {
        size_t capacity = 1024;
        int* selected = malloc(capacity * sizeof(int));
        int selectedCount = 0;
        for (int i = 0; i < 4000000; i++) {
            int n = numbers[i];
            if (n % divisor != 0) continue;
            n = n * factor + 1;
            if (n % 2 != 0) continue;
            if ((size_t)selectedCount == capacity) {
                capacity *= 2;
                selected = realloc(selected, capacity * sizeof(int));
            }
            selected[selectedCount++] = n;
        }
        for (int i = 0; i < selectedCount; i++)
            total += selected[i];
        free(selected);
    }
    int offset = 1;
    for (int i = 0; i < 8000000; i++)
        total += i * factor + offset;
    printf("%lld\n", total);
    free(numbers);
    return 0;
}
