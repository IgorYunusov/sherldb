#ifndef __LCOMMAND_H__
#define __LCOMMAND_H__

#include "ldb.h"

typedef int (*cmd_handler)(struct ldb_context *lctx, const char *cmdbuffer);

struct cmd_node {
   char *cmd;
   struct cmd_node *next;
};

struct cmd_queue {
   struct cmd_node *head;
   struct cmd_node *tail;
   int count;
};

void init_cmd_history(struct cmd_queue *queue);
void add2cmd_history(struct cmd_queue *queue, const char *cmd, int size);
struct cmd_node* get_last_cmd(struct cmd_queue *queue);

struct lcommand {
    const char *short_name;
    const char *name;
    const char *help;
    cmd_handler handler; 
};

#define CMD_NEXT (1<<0)
#define CMD_STEP (1<<1)
#define CMD_CONTINUE (1<<2)
#define CMD_BREAK (1<<3)

//void ldb_output();
void command_loop(struct ldb_context *lctx);
int cmd_debugshow(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_help(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_quit(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_loadluafile(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_break(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_delete_break(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_run(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_setargs(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_continue(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_next(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_step(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_list(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_print(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_backtrace(struct ldb_context *lctx, const char *cmdbuffer);
int cmd_set_environment(struct ldb_context *lctx, const char *cmdbuffer);

void print_luavar(lua_State *L, int index, int layer);
void print_luatable(lua_State *L, int index, int layer);

#endif
