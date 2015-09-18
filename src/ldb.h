#ifndef __LDB_H__
#define __LDB_H__

#include <lua.h>
#include <lstate.h>
#include <stdbool.h>
#include "common_def.h"
#include "lfile.h"

enum APP_RUN_MODE {
    NO_PARAM = 1,
    SINGLE_LUA_FIEL,

    MODE_INVALID,
};

struct ldb_breakpoint {
    int type;
    int line;
    int hitted_count;
    bool avaliable;
    struct ldb_filebuffer *filebuffer;
};

enum DEBUG_PROG_STATE {
    RUNNING = 1,
    TERMINATE = 2,
};

enum CMD_HANDLER_RET {
    CMD_OK = 1,
    CMD_ERR,
    CMD_QUIT,
};

struct lua_program {
    int status;
    char abspath[PATH_MAX_SIZE];
    char args[COMMON_MAX_STRING_SIZE];
    lua_State *L;
    lua_Debug *ar;
    struct CallInfo *curr_ci;
};

struct env_var {
    char *name;
    char *value;
};


struct ldb_context {
    enum APP_RUN_MODE mode;
    char prompt[32];
    char title[128];
    char *lua_file;
    bool bquit;
    char src_dir[PATH_MAX_SIZE];
    struct cmd_queue *cmd_history;

    struct lua_program *lprog;
    struct ldb_breakpoint bkt_list[MAX_BREAK_POINT_COUNT];
    int bkt_num;
    struct ldb_filebuffer fb_list[MAX_LOAD_FILE_COUNT];
    int fb_num;
    struct env_var *environ_list;
    int env_num;
    int cmd_mask;

};

bool ldb_init(struct ldb_context *lctx);
void ldb_uninit(struct ldb_context *lctx);
void debug_showctx(struct ldb_context *lctx);

#endif  
