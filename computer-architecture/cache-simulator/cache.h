#ifndef CACHE_H
#define CACHE_H

typedef unsigned char uchar;
typedef struct { int valid; long int tag; int frequency; uchar *block; } cache_line_t;
typedef struct { uchar s,t,b,E; cache_line_t **cache; } cache_t;
cache_t initialize_cache(uchar s, uchar t, uchar b, uchar E);
uchar read_byte(cache_t cache, uchar *start, long int off);
#endif
