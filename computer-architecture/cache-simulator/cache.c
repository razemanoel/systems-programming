#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"

cache_t initialize_cache(uchar s, uchar t, uchar b, uchar E) {
    cache_t cache;
    cache.s = s;
    cache.t = t;
    cache.b = b;
    cache.E = E;
    int S = 1 << s;
    cache.cache = (cache_line_t**) malloc(S * sizeof (cache_line_t*));
    for (int i = 0; i < S; ++i) {
        cache.cache[i] = (cache_line_t*) malloc(E * sizeof (cache_line_t));
        for (int j = 0; j < E; ++j) {
            cache.cache[i][j].valid = 0;
            cache.cache[i][j].frequency = 0;
            cache.cache[i][j].tag = -1;
            cache.cache[i][j].block = (uchar*)malloc((1 << b) * sizeof(uchar));
            memset(cache.cache[i][j].block, 0, (1 << b));
        }
    }
    return cache;
}
uchar read_byte(cache_t cache, uchar* start, long int off) {
    int S = 1<<cache.s;
    int B = 1 << cache.b;
    int set_index = (off >> cache.b) & (S - 1);
    long int tag = off >> (cache.s + cache.b);
    int block_offset = off & (B - 1);
    cache_line_t* set = cache.cache[set_index];
    for (int i = 0; i < cache.E ; ++i) {
        if (tag == set[i].tag && set[i].valid)
            return set[i].block[block_offset];
    }
    int min_freq = 0;
    for (int i = 0; i < cache.E; ++i) {
        if(set[i].frequency < set[min_freq].frequency)
            min_freq = i;
    }
    set[min_freq].valid = 1;
    set[min_freq].frequency = 1;
    set[min_freq].tag = tag;
    memcpy(set[min_freq].block, &start[off - block_offset], B);
    return set[min_freq].block[block_offset];
}
