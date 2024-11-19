#include <stdio.h>
#include <stdlib.h>
#include <time.h>

time_t seed = 0;

void freez(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

void _seed() {
    if (seed) {
        return;
    }
    seed = time(NULL);
    srand(seed); // set random seed
}

/**
 * generates random int between min..max
 */
int rand_range(int min, int max) {
    _seed();
    return (rand() % (max - min + 1)) + min;
}

float rand_range_f(float min, float max) {
    float scale = rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

void rand_str(char *dest, size_t len) {
    _seed();

    static const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    size_t idx;

    dest[0] = 0;
    for (size_t i = 0; i < len; i++) {
        idx = rand() % 61;
        dest[i] = charset[idx];
    }
    dest[len] = 0;
}

float clamp_f(float val, float min, float max) {
    float ret = (val < min) ? min : val;
    return (ret > max) ? max : ret;
}
