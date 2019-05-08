#include<string.h>
#include<stdio.h>
#include "cache_associative_nway.h"


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
    struct cacheLine cacheLine2;
};

static struct cacheSet cache[16];

void cache_associative_nway_init() {
    hits = 0;
    misses = 0;

    for (int a = 0; a < 16; a++) {
        cache[a].cacheLine1.counter = 0;
        cache[a].cacheLine1.valid = 0;
        cache[a].cacheLine1.tag = 0;
        cache[a].cacheLine1.dirty = 0;

        cache[a].cacheLine2.counter = 0;
        cache[a].cacheLine2.valid = 0;
        cache[a].cacheLine2.tag = 0;
        cache[a].cacheLine2.dirty = 0;

        for (int b = 0; b < 16; b++) {
            cache[a].cacheLine1.block[b] = 0;
            cache[a].cacheLine2.block[b] = 0;
        }
    }
}

int cache_associative_nway_load(memory_address addr) {
    int offset = addr & 0xF;
    int indSet = (addr >> 4) & 0xF;
    int tag = (addr >> 8) & 0xFFFFFF;

    int result = 0;

    if (cache[indSet].cacheLine1.valid == 1 && cache[indSet].cacheLine1.tag == tag) {
        hits++;
        memcpy(&result, &cache[indSet].cacheLine1.block[offset], 4);
        return result;
    } else if (cache[indSet].cacheLine2.valid == 1 && cache[indSet].cacheLine2.tag == tag) {
        hits++;
        memcpy(&result, &cache[indSet].cacheLine2.block[offset], 4);
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
        } else if (cache[indSet].cacheLine2.valid == 0) {
            cache[indSet].cacheLine2.valid = 1;
            cache[indSet].cacheLine2.tag = tag;
            storage_load_line(addr, cache[indSet].cacheLine2.block);
            memcpy(&result, &cache[indSet].cacheLine2.block[offset], 4);
            cache[indSet].cacheLine2.counter++;
            return result;
        } else if (cache[indSet].cacheLine1.counter < cache[indSet].cacheLine2.counter) {
            if (cache[indSet].cacheLine1.dirty == 1) {
                int tag = cache[indSet].cacheLine1.tag;
                tag &= 0xFFFFFF;
                int set = indSet;
                set &= 0xF;
                int add = 0;
                add |= (tag << 8);
                add |= (set << 4);

                storage_store_line(add, cache[indSet].cacheLine1.block);
                cache[indSet].cacheLine1.dirty = 0;
            }

            cache[indSet].cacheLine1.valid = 1;
            cache[indSet].cacheLine1.tag = tag;
            storage_load_line(addr, cache[indSet].cacheLine1.block);
            memcpy(&result, &cache[indSet].cacheLine1.block[offset], 4);
            cache[indSet].cacheLine1.counter++;
            return result;
        } else {
            if (cache[indSet].cacheLine2.dirty == 1) {
                int tag = cache[indSet].cacheLine2.tag;
                tag &= 0xFFFFFF;
                int set = indSet;
                set &= 0xF;
                int add = 0;
                add |= (tag << 8);
                add |= (set << 4);
                storage_store_line(add, cache[indSet].cacheLine2.block);
                cache[indSet].cacheLine2.dirty = 0;
            }
            cache[indSet].cacheLine2.valid = 1;
            cache[indSet].cacheLine2.tag = tag;
            storage_load_line(addr, cache[indSet].cacheLine2.block);
            memcpy(&result, &cache[indSet].cacheLine2.block[offset], 4);
            cache[indSet].cacheLine2.counter++;
            return result;
        }
    }
}

void cache_associative_nway_store(memory_address addr, int value) {

    int offset = addr & 0xF;
    int indSet = (addr >> 4) & 0xF;
    int tag = (addr >> 8) & 0xFFFFFF;
    char c1 = value & 0xFF;
    char c2 = (value >> 8) & 0xFF;
    char c3 = (value >> 16) & 0xFF;
    char c4 = (value >> 24) & 0xFF;

    if (cache[indSet].cacheLine1.valid == 1 && cache[indSet].cacheLine1.tag == tag) {
        hits++;

        cache[indSet].cacheLine1.dirty = 1;

        cache[indSet].cacheLine1.block[offset] = c1;
        cache[indSet].cacheLine1.block[offset + 1] = c2;
        cache[indSet].cacheLine1.block[offset + 2] = c3;
        cache[indSet].cacheLine1.block[offset + 3] = c4;

    } else if (cache[indSet].cacheLine2.valid == 1 && cache[indSet].cacheLine2.tag == tag) {
        hits++;

        cache[indSet].cacheLine2.dirty = 1;

        cache[indSet].cacheLine2.block[offset] = c1;
        cache[indSet].cacheLine2.block[offset + 1] = c2;
        cache[indSet].cacheLine2.block[offset + 2] = c3;
        cache[indSet].cacheLine2.block[offset + 3] = c4;
    } else {
        misses++;

        if (cache[indSet].cacheLine1.valid == 0) {
            storage_load_line(addr, cache[indSet].cacheLine1.block);

            cache[indSet].cacheLine1.block[offset] = c1;
            cache[indSet].cacheLine1.block[offset + 1] = c2;
            cache[indSet].cacheLine1.block[offset + 2] = c3;
            cache[indSet].cacheLine1.block[offset + 3] = c4;

            cache[indSet].cacheLine1.tag = tag;
            cache[indSet].cacheLine1.dirty = 1;
            cache[indSet].cacheLine1.valid = 1;
            cache[indSet].cacheLine1.counter++;
        } else if (cache[indSet].cacheLine2.valid == 0) {
            storage_load_line(addr, cache[indSet].cacheLine2.block);

            cache[indSet].cacheLine2.block[offset] = c1;
            cache[indSet].cacheLine2.block[offset + 1] = c2;
            cache[indSet].cacheLine2.block[offset + 2] = c3;
            cache[indSet].cacheLine2.block[offset + 3] = c4;

            cache[indSet].cacheLine2.tag = tag;
            cache[indSet].cacheLine2.dirty = 1;
            cache[indSet].cacheLine2.valid = 1;
            cache[indSet].cacheLine2.counter++;

        } else if (cache[indSet].cacheLine1.counter < cache[indSet].cacheLine2.counter) {
            if (cache[indSet].cacheLine1.dirty == 1) {
                int tag = cache[indSet].cacheLine1.tag;
                tag &= 0xFFFFFF;
                int set = indSet;
                set &= 0xF;
                int add = 0;
                add |= (tag << 8);
                add |= (set << 4);
                storage_store_line(add, cache[indSet].cacheLine1.block);
                cache[indSet].cacheLine1.dirty = 0;
            }
            storage_load_line(addr, cache[indSet].cacheLine1.block);

            cache[indSet].cacheLine1.block[offset] = c1;
            cache[indSet].cacheLine1.block[offset + 1] = c2;
            cache[indSet].cacheLine1.block[offset + 2] = c3;
            cache[indSet].cacheLine1.block[offset + 3] = c4;

            cache[indSet].cacheLine1.tag = tag;
            cache[indSet].cacheLine1.dirty = 1;
            cache[indSet].cacheLine1.valid = 1;
            cache[indSet].cacheLine1.counter++;

        } else {
            if (cache[indSet].cacheLine2.dirty == 1) {
                int tag = cache[indSet].cacheLine2.tag;
                tag &= 0xFFFFFF;
                int set = indSet;
                set &= 0xF;
                int add = 0;
                add |= (tag << 8);
                add |= (set << 4);

                storage_store_line(add, cache[indSet].cacheLine2.block);
                cache[indSet].cacheLine2.dirty = 0;
            }
            storage_load_line(addr, cache[indSet].cacheLine2.block);
            cache[indSet].cacheLine2.block[offset] = c1;
            cache[indSet].cacheLine2.block[offset + 1] = c2;
            cache[indSet].cacheLine2.block[offset + 2] = c3;
            cache[indSet].cacheLine2.block[offset + 3] = c4;

            cache[indSet].cacheLine2.tag = tag;
            cache[indSet].cacheLine2.dirty = 1;
            cache[indSet].cacheLine2.valid = 1;
            cache[indSet].cacheLine2.counter++;
        }
    }
}

void cache_associative_nway_flush() {
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
        } else if (cache[i].cacheLine2.dirty == 1) {
            int tag = cache[i].cacheLine2.tag;
            tag &= 0xFFFFFF;
            int set = i;
            set &= 0xF;
            int addr = 0;
            addr |= (tag << 8);
            addr |= (set << 4);

            storage_store_line(addr, cache[i].cacheLine2.block);
            cache[i].cacheLine2.dirty = 0;
        }
    }
}

void cache_associative_nway_stats() {
    printf("Cache N-Way Statistics:\t-Hits: %d\t-Misses: %d\n", hits, misses);
}

