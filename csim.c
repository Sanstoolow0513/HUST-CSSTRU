#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

typedef struct {
    int valid;
    unsigned long long tag;
    unsigned long long last_used;
} cache_line;

int s = 0, E = 0, b = 0;
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;
unsigned long long current_timestamp = 0;

cache_line** cache;

void init_cache() {
    int S = 1 << s;
    cache = (cache_line**)malloc(S * sizeof(cache_line*));
    for (int i = 0; i < S; i++) {
        cache[i] = (cache_line*)calloc(E, sizeof(cache_line));
    }
}

void access_memory(unsigned long long address) {
    current_timestamp++;

    unsigned long long tag = address >> (s + b);
    unsigned long long set_index = (address >> b) & ((1 << s) - 1);

    cache_line* lines = cache[set_index];

    int hit = 0;
    for (int i = 0; i < E; i++) {
        if (lines[i].valid && lines[i].tag == tag) {
            lines[i].last_used = current_timestamp;
            hit = 1;
            break;
        }
    }

    if (hit) {
        hit_count++;
        return;
    }

    miss_count++;

    for (int i = 0; i < E; i++) {
        if (!lines[i].valid) {
            lines[i].valid = 1;
            lines[i].tag = tag;
            lines[i].last_used = current_timestamp;
            return;
        }
    }

    eviction_count++;
    int lru_idx = 0;
    unsigned long long min_time = lines[0].last_used;
    for (int i = 1; i < E; i++) {
        if (lines[i].last_used < min_time) {
            min_time = lines[i].last_used;
            lru_idx = i;
        }
    }
    lines[lru_idx].tag = tag;
    lines[lru_idx].last_used = current_timestamp;
}

void free_cache() {
    int S = 1 << s;
    for (int i = 0; i < S; i++) {
        free(cache[i]);
    }
    free(cache);
}

int main(int argc, char *argv[]) {
    int opt;
    char *trace_file = NULL;

    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-s s] [-E E] [-b b] [-t trace]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (s <= 0 || E <= 0 || b <= 0 || trace_file == NULL) {
        fprintf(stderr, "Invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    init_cache();

    FILE *fp = fopen(trace_file, "r");
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        char op;
        unsigned long long address;
        int size;

        if (sscanf(line, " %c %llx,%d", &op, &address, &size) < 3) {
            continue;
        }

        if (op == 'I') {
            continue;
        }

        if (op == 'L' || op == 'S') {
            access_memory(address);
        } else if (op == 'M') {
            access_memory(address);
            access_memory(address);
        }
    }

    fclose(fp);
    free_cache();

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}