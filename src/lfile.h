#ifndef __LFILE_H__
#define __LFILE_H__

#include <stdbool.h>
#include "common_def.h"

struct ldb_filebuffer {
    bool is_loaded;
    char abspath[PATH_MAX_SIZE];
    int line_count;
    char **lines;
};

int load_sourcefile(struct ldb_filebuffer *filebuffer, const char *path);

#endif
