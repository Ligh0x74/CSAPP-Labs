#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "cachelab.h"

char *file;
struct Line *cache;
int v, s, E, b, S, B;
int hits, misses, evictions;

// 打印帮助信息
void printHelp(char **argv) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n"
           "Options:\n"
           "  -h         Print this help message.\n"
           "  -v         Optional verbose flag.\n"
           "  -s <num>   Number of set index bits.\n"
           "  -E <num>   Number of lines per set.\n"
           "  -b <num>   Number of block offset bits.\n"
           "  -t <file>  Trace file.\n"
           "\n"
           "Examples:\n"
           "  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n"
           "  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0], argv[0], argv[0]);
}

// 处理命令行参数
void handleParam(int argc, char **argv) {
    while (1) {
        int opt = getopt(argc, argv, "hvs:E:b:t:");
        if (opt == -1) {
            printHelp(argv);
            break;
        }
        switch (opt) {
            case 'h':
                printHelp(argv);
                exit(0);
            case 'v':
                v = 1;
                break;
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
                file = optarg;
                break;
            default:
                printHelp(argv);
                exit(1);
        }
    }
}

// 省略每行的数据部分
// 不能使用time函数,毫秒级的时间戳粒度太大
// 所以使用计数的方式定义存活时间
struct Line {
    int valid;
    long tag;
    long time;
};

// 初始化Cache
void initialCache() {
    S = 1 << s;
    B = 1 << b;
    long lineCount = S * E;
    cache = malloc(lineCount * sizeof(struct Line));
    for (long i = 0; i < lineCount; i++) {
        cache[i].valid = 0;
        cache[i].tag = 0;
        cache[i].time = 0;
    }
}

// 更新time
void updateTime(long lineCount) {
    long index;
    for (long i = 0; i < E; i++) {
        index = lineCount + i;
        if (cache[index].valid) {
            cache[index].time++;
        }
    }
}

// LRU算法
void lru(long lineCount, long tag) {
    long curIndex, maxIndex = lineCount;
    for (long i = 1; i < E; i++) {
        curIndex = lineCount + i;
        if (cache[curIndex].time > cache[maxIndex].time) {
            maxIndex = curIndex;
        }
    }
    cache[maxIndex].tag = tag;
    cache[maxIndex].time = 0;
}

// 模拟内存访问
void memoryAccess() {
    FILE *fp = fopen(file, "r");
    char operation;
    long address, size;
    while (fscanf(fp, " %c %lx,%ld", &operation, &address, &size) != EOF) {
        if (operation == 'I') {
            continue;
        }
        long tag = address >> (s + b);
        long set = (address >> b) & (S - 1);
        long lineCount = set * E;
        for (long i = 0; i < E; i++) {
            long index = lineCount + i;
            if (cache[index].valid && cache[index].tag == tag) {
                hits++;
                cache[index].time = 0;
                if (v) printf("%c %lx,%ld hit", operation, address, size);
                break;
            } else if (cache[index].valid == 0) {
                misses++;
                cache[index].valid = 1;
                cache[index].tag = tag;
                if (v) printf("%c %lx,%ld miss", operation, address, size);
                break;
            } else if (i == E - 1) {
                misses++;
                evictions++;
                lru(lineCount, tag);
                if (v) printf("%c %lx,%ld miss eviction", operation, address, size);
                break;
            }
        }
        if (operation == 'M') {
            hits++;
            if (v) printf(" hit");
        }
        printf("\n");
        updateTime(lineCount);
    }
}

int main(int argc, char **argv) {
    handleParam(argc, argv);
    initialCache();
    memoryAccess();
    printSummary(hits, misses, evictions);
    return 0;
}
