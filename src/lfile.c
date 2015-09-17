#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "lfile.h"

int load_sourcefile(struct ldb_filebuffer *filebuffer, const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "ldb load source file %s failed!", path);
        return 1;
    }

    char line_buff[MAX_LINE_SIZE];
    int line_num = 1;
    int capacity = 32;
    int line_size;

    filebuffer->lines = (char **)malloc(capacity * sizeof(char *));
    while(fgets(line_buff, MAX_LINE_SIZE, f)) {
        line_size = strlen(line_buff);
        filebuffer->lines[line_num] = (char *)malloc(line_size*sizeof(char) + 1);
        strcpy(filebuffer->lines[line_num], line_buff);
        line_num++;
        if (line_num > capacity) {
            char **tmp = filebuffer->lines;
            capacity *= 2;
            filebuffer->lines = (char **)realloc(filebuffer->lines, capacity*sizeof(char*));
            if(filebuffer->lines == NULL) {
                free(tmp);
                return 1;
            }
        }
    }

    strcpy(filebuffer->abspath, path);
    filebuffer->line_count = line_num - 1;
    return 0;
}
