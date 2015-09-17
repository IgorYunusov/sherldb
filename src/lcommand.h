#ifndef __LCOMMAND_H__
#define __LCOMMAND_H__

#include "ldb.h"

typedef int (*cmd_handler)(struct ldb_context *lctx, const char *cmdbuffer);

void command_loop(struct ldb_context *lctx);

struct lcommand {
    const char *short_name;
    const char *name;
    const char *help;
    cmd_handler handler; 
};

int cmd_debugshow(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_help(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_quit(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_loadluafile(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_break(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_run(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_setargs(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_continue(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_next(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_step(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_list(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_backtrace(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_set_environment(struct ldb_context *lctx, const char *cmdbuffer);

#endif
