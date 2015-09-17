#include <stdio.h>
#include <string.h>
#include "ldb.h"
#include "lcommand.h"

int parse_args(int argc, char **argv, struct ldb_context* lctx) {
    if (argc < 2 ) {
        printf("not enough params!\n");
        return 1;
    } else {
        int i;
        char args[COMMON_MAX_STRING_SIZE] = {0};

        cmd_loadluafile(lctx, argv[1]);
        for(i=2; i<argc; i++) {
            strcat(args, argv[i]);
            strcat(args, " ");
        }
        cmd_setargs(lctx, args); 
    }
    return 0;
}

int main(int argc, char **argv) {
    struct ldb_context lctx;
    if (!ldb_init(&lctx)) {
        return -1;
    }
    
    if (parse_args(argc, argv, &lctx) != 0) {
        return 1;
    }
    printf("%s\n", lctx.title);
    command_loop(&lctx); 

    return 0;
}
