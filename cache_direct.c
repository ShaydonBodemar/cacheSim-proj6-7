#include<string.h>
#include<stdio.h>
#include "cache_direct.h"


static int hits = 0;
static int misses = 0;

struct cacheLine {
    int counter;
    int valid;
    int tag;
    int dirty;
    cache_line block;
};

struct cacheSet {
    struct cacheLine cacheLine1;
};

static struct cacheSet cache[16];

void cache_direct_init() {
    hits = 0;
    misses = 0;

    for (int i = 0; i < 16; i++) {
        cache[i].cacheLine1.counter = 0;
        cache[i].cacheLine1.valid = 0;
        cache[i].cacheLine1.tag = 0;
        cache[i].cacheLine1.dirty = 0;
    }
}

int cache_direct_load(memory_address addr) {
    int offset = addr & 0xF;
    int indSet = (addr >> 4) & 0xF;
    int tag = (addr >> 8) & 0xFFFFFF;

    int result = 0;

    if (cache[indSet].cacheLine1.valid == 1 && cache[indSet].cacheLine1.tag == tag) {
        hits++;
        memcpy(&result, &cache[indSet].cacheLine1.block[offset], 4);
        return result;
    } else {
        misses++;

        if (cache[indSet].cacheLine1.valid == 0) {
            cache[indSet].cacheLine1.valid = 1;
            cache[indSet].cacheLine1.tag = tag;
            storage_load_line(addr, cache[indSet].cacheLine1.block);
            memcpy(&result, &cache[indSet].cacheLine1.block[offset], 4);
            cache[indSet].cacheLine1.counter++;
            return result;
        }
        cache[indSet].cacheLine1.valid = 1;
        cache[indSet].cacheLine1.tag = tag;
        storage_load_line(addr, cache[indSet].cacheLine1.block);
        memcpy(&result, &cache[indSet].cacheLine1.block[offset], 4);
        cache[indSet].cacheLine1.counter++;
        return result;
    }
}

void cache_direct_store(memory_address addr, int value) {

    int offset = addr & 0xF;
    int indSet = (addr >> 4) & 0xF;
    int tag = (addr >> 8) & 0xFFFFFF;

    /*char c1 = value & 0xFF;
    char c2 = (value >> 8) & 0xFF;
    char c3 = (value >> 16) & 0xFF;
    char c4 = (value >> 24) & 0xFF;*/

    if (cache[indSet].cacheLine1.valid == 1 && cache[indSet].cacheLine1.tag == tag) {
        hits++;

        cache[indSet].cacheLine1.dirty = 1;

        /*cache[indSet].cacheLine1.block[offset] = c1;
        cache[indSet].cacheLine1.block[offset + 1] = c2;
        cache[indSet].cacheLine1.block[offset + 2] = c3;
        cache[indSet].cacheLine1.block[offset + 3] = c4;*/
        cache[indSet].cacheLine1.block[offset] = value;

    } else {
        misses++;

        if (cache[indSet].cacheLine1.valid == 0) {
            storage_load_line(addr, cache[indSet].cacheLine1.block);

            /*cache[indSet].cacheLine1.block[offset] = c1;
            cache[indSet].cacheLine1.block[offset + 1] = c2;
            cache[indSet].cacheLine1.block[offset + 2] = c3;
            cache[indSet].cacheLine1.block[offset + 3] = c4;*/
            cache[indSet].cacheLine1.block[offset] = value;

            cache[indSet].cacheLine1.tag = tag;
            cache[indSet].cacheLine1.dirty = 1;
            cache[indSet].cacheLine1.valid = 1;
            cache[indSet].cacheLine1.counter++;
        }
    }
}

void cache_direct_flush() {
    for (int i = 0; i < 16; i++) {
        if (cache[i].cacheLine1.dirty == 1) {
            int tag = cache[i].cacheLine1.tag;
            tag &= 0xFFFFFF;
            int set = i;
            set &= 0xF;
            int addr = 0;
            addr |= (set << 4);
            addr |= (tag << 8);

            storage_store_line(addr, cache[i].cacheLine1.block);
            cache[i].cacheLine1.dirty = 0;
        }
    }
}

void cache_direct_stats() {
    printf("Cache Direct Statistics:\t-Hits: %d\t-Misses: %d\n", hits, misses);
}