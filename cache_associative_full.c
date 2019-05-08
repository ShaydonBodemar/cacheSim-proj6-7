#include<string.h>
#include<stdio.h>
#include "cache_associative_full.h"

static int hits = 0;
static int misses = 0;

struct cacheLine {
    int counter;
    int valid;
    int tag;
    int dirty;
    cache_line block;
};

static struct cacheLine cache[16];

void cache_associative_full_init() {
    hits = 0;
    misses = 0;

    for (int a = 0; a < 16; a++) {
        cache[a].counter = 0;
        cache[a].valid = 0;
        cache[a].tag = -1;
        cache[a].dirty = 0;

        for (int b = 0; b < 16; b++)
            cache[a].block[b] = 0;
    }
}

int cache_associative_full_load(memory_address addr) {
    int offset = addr & 0xF;
    int tag = (addr >> 4) & 0xFFFFFFF;
    int result;

    for (int i = 0; i < 16; i++) {
        if (cache[i].valid == 1 && cache[i].tag == tag) {
            hits++;
            //adding an inrecement for logic testing
            //cache[i].counter++;
            memcpy(&result, &cache[i].block[offset], 4);
            return result;
        }
    }

    misses++;

    for (int i2 = 0; i2 < 16; i2++) {
        if (cache[i2].valid == 0) {
            cache[i2].valid = 1;
            cache[i2].tag = tag;
            storage_load_line(addr, cache[i2].block);
            memcpy(&result, &cache[i2].block[offset], 4);
            cache[i2].counter++;
            return result;
        }
    }

    int index = 0;
    for (int i3 = 1; i3 < 16; i3++) {
        //comparison sign was flipped from < to > for logic testing
        if (cache[i3].counter < cache[i3 - 1].counter)
            index = i3;
    }

    /*if (cache[index].dirty == 1) {
        int tag = cache[index].tag;
        tag &= 0xFFFFFFF;
        int add = 0;
        add |= (tag << 4);

        storage_store_line(add, cache[index].block);
        cache[index].dirty = 0;
    }*/

    cache[index].valid = 1;
    cache[index].tag = tag;
    storage_load_line(addr, cache[index].block);
    memcpy(&result, &cache[index].block[offset], 4);
    cache[index].counter++;
    return result;
}

void cache_associative_full_store(memory_address addr, int value) {
    int offset = addr & 0xF;
    int tag = (addr >> 4) & 0xFFFFFFF;

    /*char c1 = value & 0xFF;
    char c2 = (value >> 8) & 0xFF;
    char c3 = (value >> 16) & 0xFF;
    char c4 = (value >> 24) & 0xFF;*/

    for (int i = 0; i < 16; i++) {
        if (cache[i].valid == 1 && cache[i].tag == tag) {
            hits++;
            //adding an inrecement for logic testing
            //cache[i].counter++;

            cache[i].dirty = 1;

            /*cache[i].block[offset] = c1;
            cache[i].block[offset + 1] = c2;
            cache[i].block[offset + 2] = c3;
            cache[i].block[offset + 3] = c4;*/
            cache[i].block[offset] = value;

            return;
        }
    }

    misses++;

    for (int i2 = 0; i2 < 16; i2++) {
        if (cache[i2].valid == 0) {
            cache[i2].valid = 1;
            cache[i2].tag = tag;

            storage_load_line(addr, cache[i2].block);

            /*cache[i2].block[offset] = c1;
            cache[i2].block[offset + 1] = c2;
            cache[i2].block[offset + 2] = c3;
            cache[i2].block[offset + 3] = c4;*/
            cache[i2].block[offset] = value;

            cache[i2].dirty = 1;
            cache[i2].counter++;
            return;
        }
    }

    int index = 0;
    for (int i3 = 1; i3 < 16; i3++) {
        //comparison sign was flipped from < to > for logic testing
        if (cache[i3].counter < cache[i3 - 1].counter)
            index = i3;
    }

    if (cache[index].dirty == 1) {
        int tag = cache[index].tag;
        tag &= 0xFFFFFFF;
        int add = 0;
        add |= (tag << 4);

        //changing from storage_store_line to storage_store in order to test logic of storage
        //changed from add to addr in parameters
        //storage_store(addr, value);
        storage_store_line(add, cache[index].block);
        cache[index].dirty = 0;
    }

    storage_load_line(addr, cache[index].block);

    /*cache[index].block[offset] = c1;
    cache[index].block[offset + 1] = c2;
    cache[index].block[offset + 2] = c3;
    cache[index].block[offset + 3] = c4;*/
    cache[index].block[offset] = value;

    cache[index].tag = tag;
    cache[index].dirty = 1;
    cache[index].valid = 1;
    cache[index].counter++;
}

void cache_associative_full_flush() {
    for (int i = 0; i < 16; i++) {
        if (cache[i].dirty == 1) {
            int tag = cache[i].tag;
            tag &= 0xFFFFFFF;
            int addr = 0;
            addr |= (tag << 4);

            storage_store_line(addr, cache[i].block);
            cache[i].dirty = 0;
        }
    }
}

void cache_associative_full_stats() {
    printf("Cache Fully Associative Statistics:\t-Hits: %d\t-Misses: %d\n", hits, misses);
}

