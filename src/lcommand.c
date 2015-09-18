#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lstate.h>
#include <lauxlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "ldb.h"
#include "lfile.h"
#include "lcommand.h"

const char COPYRIGHT[] = "Copyright (C) 2015 free software";
const char VERSION[] = "version 0.1";
const char APP_NAME[] = "sherldb";
const char AUTHOR[] = "sherlook";
const char PROMPT[] = "(ldb)";
const char REGISTRY_TAG[] = "__ldb_udata";

static struct lcommand lcmds[] = {
    {
        "", 
        "debugshow",
        "just for debug using",
        &cmd_debugshow,
    },
    {
        "h",
        "help",
        "help info",
        &cmd_help,
    },
    {
        "q",
        "quit",
        "quit the debugger",
        &cmd_quit,
    },
    {
        "load",
        "loadfile",
        "load the lua file",
        &cmd_loadluafile,
    },
    {
        "b",
        "break",
        "Set break point",
        &cmd_break,
    },
    {
        "r", 
        "run",
        "exec lua file",
        &cmd_run,
    },
    {
        "c",
        "continue",
        "Continue exec until net next break point",
        &cmd_continue,
    },
    {
        "n",
        "next",
        "Step program, preceeding through subroutine calls.",
        &cmd_next,
    },
    {
        "s",
        "step",
        "Step program until it reacheds a different source line.",
        &cmd_step,
    },
    {
        "l",
        "list",
        "List the source code",
        &cmd_list,
    },
    {
        "bt",
        "backtrace",
        "Print backtrace of all stack frames",
        &cmd_backtrace,
    },
    {
        "set env",
        "set environment",
        "set the environment during the debug time",
        &cmd_set_environment,
    },
};

void debug_showctx(struct ldb_context *lctx) {
    int i;
    printf("lctx src dir:%s\n", lctx->src_dir);
    printf("lctx lua_program:%s\n", lctx->lprog->abspath);
    printf("lctx lua_args:%s\n", lctx->lprog->args);
    
    printf("lctx break point list:%d\n", lctx->bkt_num);
    for(i=0; i<lctx->bkt_num; i++) {
        struct ldb_filebuffer *fb = lctx->bkt_list[i].filebuffer;
        printf("\tfile:%s line:%d\n", fb->abspath, lctx->bkt_list[i].line);
    }
    printf("lctx environ list:%d\n", lctx->env_num);
    for(i=0; i<lctx->env_num; i++) {
        printf("%s=%s\n", lctx->environ_list[i].name, lctx->environ_list[i].value);
    }
}

bool ldb_init(struct ldb_context *lctx) {
    int capacity;
    struct lua_program *lp = (struct lua_program*)malloc(sizeof(struct lua_program));
    lp->L = NULL;
    lp->ar = NULL;
    lp->status = TERMINATE;
    memset(lp->abspath, 0, PATH_MAX_SIZE);
    memset(lp->args, 0, PATH_MAX_SIZE);

    lctx->lprog = lp;
    sprintf(lctx->title, "%s, %s", APP_NAME, "a simple gdb-like lua debugger");
    strcpy(lctx->prompt, PROMPT);
    lctx->bquit = false;
    lctx->bkt_num = 0;
    lctx->fb_num = 0;
    lctx->cmd_mask = 0;

    capacity = 256;
    lctx->environ_list = (struct env_var*)malloc(capacity*sizeof(struct env_var));
    lctx->env_num = 0; 
    if (getcwd(lctx->src_dir, PATH_MAX_SIZE) == NULL) {
        printf("Error: getcwd return NULL");
        return false;
    }

    lctx->cmd_history = (struct cmd_queue*)malloc(sizeof(struct cmd_queue));
    init_cmd_history(lctx->cmd_history);

    return true;
}

void ldb_uninit(struct ldb_context *lctx) {
    if (lctx->lprog->L) {
        //lua_close(lctx->lprog->L);
    }
}

cmd_handler _get_cmd_handler(char *cmd) {
    int i;
    for(i = 0; i < sizeof(lcmds)/sizeof(lcmds[0]); ++i) {
        if (strcmp(lcmds[i].short_name, "") == 0) {
            continue;
        }
        if(strcmp(lcmds[i].short_name, cmd) == 0 || strcmp(lcmds[i].name, cmd) == 0) {
            return lcmds[i].handler;
        }
    }
    return NULL;
}

void command_loop(struct ldb_context *lctx) {
    int cmd_handler_ret;
    //struct lua_program *lp;
    while(!lctx->bquit) {
        char *readbuff = readline(lctx->prompt); 
        int len = strlen(readbuff);
        if (len == 0) {
            continue;
        }
        char *cmdbuffer = readbuff;
        char *token = strsep(&cmdbuffer, " ");
        if (token == NULL) {
            token = cmdbuffer;
        }
        add2cmd_history(lctx->cmd_history, token, len);
        cmd_handler fn = _get_cmd_handler(token); 
        if (fn) {
            cmd_handler_ret = fn(lctx, cmdbuffer);
        } else {
            printf("\"%s\" unknown command\n", readbuff);
            cmd_help(lctx, NULL); 
        }
        printf("\n");
        
        if(cmd_handler_ret == CMD_QUIT) {
            break;
        }
    }

    ldb_uninit(lctx);
}

void debug_cmd_loop(struct ldb_context *lctx) {
    int cmd_ret;
    struct lua_program *lp = lctx->lprog;
    if (!lp) {
        printf("lprog is NULL\n");
        return;
    }
    while(!lctx->bquit) {
        char *readbuff = readline("(ldb test.lua)"); 
        int len = strlen(readbuff);
        if (len == 0) {
            continue;
        }
        char *cmdbuffer = readbuff;
        char *token = strsep(&cmdbuffer, " ");
        if (token == NULL) {
            token = cmdbuffer;
        }
        add2cmd_history(lctx->cmd_history, token, len);
        cmd_handler fn = _get_cmd_handler(token); 

        if(fn) {
            cmd_ret = fn(lctx, cmdbuffer);
        } else {
            printf("\"%s\" unknown command\n", readbuff);
            cmd_help(lctx, NULL); 
        }

        if(cmd_ret == CMD_QUIT) {
            break;
        }
        printf("\n");
    }
}

//int get_calldepth(lua_St
/*
 * =============================================================================
 * hook functions
 * =============================================================================
*/
int add_breakpoint(struct ldb_context *lctx, struct ldb_breakpoint bkt) {
    int i;
    for(i=0; i<lctx->bkt_num; i++) {
        if(lctx->bkt_list[i].avaliable) {
            memcpy(&lctx->bkt_list[i], &bkt, sizeof(bkt));
            return i;
        }
    }
    if( lctx->bkt_num < MAX_BREAK_POINT_COUNT) {
        memcpy(&lctx->bkt_list[lctx->bkt_num], &bkt, sizeof(bkt));
        lctx->bkt_num++;
        return lctx->bkt_num - 1;
    }
    
    //cannot add more break point
    return -1;
}

int find_breakpoint(struct ldb_context *lctx, const char *abspath, int line) {
    int i = 0;
    for(; i<lctx->bkt_num; i++) {
        struct ldb_filebuffer *fb = lctx->bkt_list[i].filebuffer;
        if(strcmp(fb->abspath, abspath) == 0 && lctx->bkt_list[i].line == line) {
            return i;
        }
    }
    return -1;
}

void line_hook(lua_State *L, lua_Debug *ar) {
    const char *file = NULL;
    struct ldb_context *lctx = NULL;
    lua_pushstring(L, REGISTRY_TAG);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lctx = (struct ldb_context *)lua_touserdata(L, -1); 

    if (!lctx) {
        printf("line hook get ldb context failed!\n");
        return;
    }
    if (lua_getinfo(L, "nSl", ar) == 0) {
        printf("line hook,lua_getinfo failed\n");
        return;
    }

    //printf("ci:%p pre:%p next:%p currentline:%d\n", ar->i_ci, ar->i_ci->previous,ar->i_ci->next, ar->currentline);
    //struct cmd_node *last = get_last_cmd(lctx->cmd_history);
    if( ar->source[0] == '@' ) {
        file = ar->source + 1;
    }
    //check next command
    struct CallInfo *curr_ci = lctx->lprog->curr_ci;
    if(file && (lctx->cmd_mask & CMD_NEXT) &&
        (curr_ci == ar->i_ci || curr_ci == ar->i_ci->next)) { 
        //printf("currci:%p pre:%p next:%p \n", curr_ci, curr_ci->previous, curr_ci->next);
        debug_cmd_loop(lctx);
        return;
    }
    //check step command
    if(file && (lctx->cmd_mask & CMD_STEP)) {
        debug_cmd_loop(lctx);
        return;
    }
    //check if hit any break point
    if (file) {
        int bkt_index = find_breakpoint(lctx, file, ar->currentline);
        if (bkt_index != -1) {
            struct ldb_filebuffer *fb = lctx->bkt_list[bkt_index].filebuffer;
            printf("Breakpoint %d, at %s:%d\n", bkt_index, fb->abspath, lctx->bkt_list[bkt_index].line);
            lctx->lprog->ar = ar;

            debug_cmd_loop(lctx);
        }
    } 
}

void return_hook(lua_State *L, lua_Debug *ar) {
    struct ldb_context *lctx = NULL;
    lua_pushstring(L, REGISTRY_TAG);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lctx = (struct ldb_context *)lua_touserdata(L, -1); 

    if(!lctx) {
        printf("return_hook get ldb context failed!\n");
        return;
    }

    if (lua_getinfo(L, "nSl", ar)) {
        printf("return hook %s %s %d\n", ar->what, ar->name, ar->currentline);
    }
}

/*
 * =============================================================================
 * command functions
 * =============================================================================
*/

void init_cmd_history(struct cmd_queue *queue) {
    queue->head = queue->tail = NULL;
    queue->count = 0;
}

void add2cmd_history(struct cmd_queue *queue, const char *cmd, int size) {
    if(queue->count < COMMAND_HISTORY_COUNT) {
        struct cmd_node *new_tail = (struct cmd_node*)malloc(sizeof(struct cmd_node));
        if (new_tail == NULL) {
            printf("add cmd history, malloc failed!\n");
            return;
        }
        new_tail->cmd = (char*)malloc(sizeof(char)*size);
        strcpy(new_tail->cmd, cmd);

        if(queue->count == 0) {
            queue->head = new_tail;
            queue->tail = new_tail;
            queue->head->next = NULL;
        }
        new_tail->next = queue->tail;
        queue->tail = new_tail;
        queue->count++;
    } else {
        struct cmd_node *old_head = queue->head;
        struct cmd_node *t = queue->tail;
        while(t->next != queue->head) t = t->next;
        queue->head = t;
        queue->head->next = NULL;
        strcpy(old_head->cmd, cmd);
        old_head->next = queue->tail;
        queue->tail = old_head;
    }
}

struct cmd_node* get_last_cmd(struct cmd_queue *queue) {
    return queue->tail;
}

int cmd_debugshow(struct ldb_context *lctx, const char *cmdbuffer) {
    debug_showctx(lctx);
    return 0;
}

int cmd_help(struct ldb_context *lctx, const char *cmdbuffer) {
    printf("help info!");
    return 0;
}

int cmd_quit(struct ldb_context *lctx, const char *cmdbuffer) {
    lctx->bquit = true;
    return 0;
}

int cmd_break(struct ldb_context *lctx, const char *cmdbuffer) {
    if(lctx->lprog->L == NULL) {
        printf("Error:no lua file loaded!\n");
        return 1;
    }
    char tmp[PATH_MAX_SIZE];
    char abspath[PATH_MAX_SIZE];
    char *cmd = tmp;
    strcpy(cmd, cmdbuffer);
    char *file = strsep(&cmd, ":");
    int i;
    struct ldb_breakpoint bkt;
    bkt.avaliable = true;
    bkt.type = LUA_MASKLINE;
    bkt.hitted_count = 0;

    if (cmd == NULL) {
        strcpy(abspath, lctx->lprog->abspath);
        bkt.line = atoi(tmp);
    } else {
        sprintf(abspath, "%s/%s", lctx->src_dir, file);
        bkt.line = atoi(cmd);
    }
    
    for(i=0; i<lctx->fb_num; i++) {
        if(strcmp(lctx->fb_list[i].abspath, abspath) == 0) {
            break;
        }
    }
    if(i == lctx->fb_num) {
        load_sourcefile(&lctx->fb_list[lctx->fb_num++], abspath);
    }    
    bkt.filebuffer = &lctx->fb_list[i];
    int index = add_breakpoint(lctx, bkt);
    if (index == -1) {
        printf("cannot add more break point\n");
    } else {
        printf("Breakpoint %d at file %s, line %d.\n", index, bkt.filebuffer->abspath, bkt.line);
    }

    return CMD_OK;
}

int cmd_continue(struct ldb_context *lctx, const char *cmdbuffer) {
    if(lctx->lprog && lctx->lprog->status == RUNNING) {
        printf("Continuing.\n");
        return CMD_QUIT;
    } else {
        printf("program not runnig!\n");
        return CMD_ERR;
    }
}

struct ldb_filebuffer *get_filebuffer(struct ldb_context *lctx, const char *abspath) {
    int i; 
    for(i=0; i<lctx->fb_num; i++) {
        if(strcmp(abspath, lctx->fb_list[i].abspath) == 0) {
            return &lctx->fb_list[i];
        }
    }
    return NULL;
}


int cmd_next(struct ldb_context *lctx, const char *cmdbuffer) {
    if (lctx->lprog->status != RUNNING) {
        printf("program not running! %d\n", lctx->lprog->status);
        return CMD_ERR;
    }
    struct ldb_filebuffer *fb = get_filebuffer(lctx, lctx->lprog->abspath);     
    int line = lctx->lprog->ar->currentline;
    printf("%d\t\t%s", line, fb->lines[line]);

    lctx->lprog->curr_ci = lctx->lprog->ar->i_ci;
    lctx->cmd_mask = CMD_NEXT;
    return CMD_QUIT;
}

int cmd_step(struct ldb_context *lctx, const char *cmdbuffer) {
    if (lctx->lprog->status != RUNNING) {
        printf("program not running! %d\n", lctx->lprog->status);
        return CMD_ERR;
    }
    struct ldb_filebuffer *fb = get_filebuffer(lctx, lctx->lprog->abspath);     
    int line = lctx->lprog->ar->currentline;
    printf("%d\t\t%s", line, fb->lines[line]);

    lctx->lprog->curr_ci = lctx->lprog->ar->i_ci;
    lctx->cmd_mask = CMD_STEP;
    return CMD_QUIT;
}

int cmd_list(struct ldb_context *lctx, const char *cmdbuffer) {
    if (lctx->lprog->ar == NULL) {
        printf("Error: cannot use command 'list' before you hit a break point\n");
        return CMD_ERR;
    }
    int list_line, i;
    lua_getinfo(lctx->lprog->L, "Sl", lctx->lprog->ar);
    if (cmdbuffer == NULL) {
        list_line = lctx->lprog->ar->currentline;
    } else {
        list_line = atoi(cmdbuffer);
    }
    
    char file[PATH_MAX_SIZE];
    if(lctx->lprog->ar->source[0] == '@') {
        strcpy(file, lctx->lprog->ar->source+1);
    }

    for(i=0; i<lctx->fb_num; i++) {
        if(strcmp(file, lctx->fb_list[i].abspath) == 0) {
            int begin = MAX(list_line - LIST_COUNT, 1);
            int end = MIN(list_line + LIST_COUNT, lctx->fb_list[i].line_count);
            int cl = begin;
            for(; cl <= end; cl++) {
                if (cl == list_line) 
                    printf("=>%d\t\t%s", cl, lctx->fb_list[i].lines[cl]);
                else
                    printf("%d\t\t%s", cl, lctx->fb_list[i].lines[cl]);
            }
            break;
        }
    }
    return CMD_OK;
}

int cmd_loadluafile(struct ldb_context *lctx, const char *filepath) {
    char abspath[PATH_MAX_SIZE];
    sprintf(abspath, "%s/%s", lctx->src_dir, filepath);
    if(access(abspath, F_OK) == -1) {
        printf("load %s failed,errno:%d\n", abspath, errno);
        return -1;
    }
    strcpy(lctx->lprog->abspath, abspath);
    lctx->lprog->L = luaL_newstate();

    lua_pushstring(lctx->lprog->L, REGISTRY_TAG);
    lua_pushlightuserdata(lctx->lprog->L, (void*)lctx);
    lua_settable(lctx->lprog->L, LUA_REGISTRYINDEX);

    int mask = lua_gethookmask(lctx->lprog->L);
    lua_sethook(lctx->lprog->L, line_hook, LUA_MASKLINE | mask, 0);
    return 0;
}

int cmd_run(struct ldb_context *lctx, const char *cmdbuffer) {
    struct lua_program *lp = lctx->lprog;
    if (lp->status == RUNNING) {
        printf("program is already running!\n");
        return CMD_ERR;
    }
    printf("Starting program: %s %s\n", lp->abspath, lp->args);
    lp->status = RUNNING;

    luaL_openlibs(lp->L);
    if(luaL_loadfile(lp->L, lp->abspath) == LUA_OK) {
        char *tmp = lp->args;
        char *arg;
        int nargs = 0;
        while((arg = strsep(&tmp, " ")) != NULL) {
            lua_pushstring(lp->L, arg);
            nargs++;
        }
        lua_pcall(lp->L, nargs, LUA_MULTRET, 0);
        printf("program end\n");
        lp->status = TERMINATE;
    }
    return CMD_OK;
}

int cmd_setargs(struct ldb_context *lctx, const char *cmdbuffer) {
    strcpy(lctx->lprog->args, cmdbuffer);
    return CMD_OK;
}

int cmd_backtrace(struct ldb_context *lctx, const char *cmdbuffer) {
    struct lua_program *lp = lctx->lprog;
    if(lp->status == RUNNING && lp->L != NULL) {
        const char *traceback;
        luaL_traceback(lp->L, lp->L, NULL, 1);
        traceback = lua_tostring(lp->L, -1); 
        printf("%s\n", traceback);
    }
    return 0;
}

int cmd_set_environment(struct ldb_context *lctx, const char *cmdbuffer) {
    char *tmp = (char*)cmdbuffer;
    char *name = strsep(&tmp, "=");
    if(name == NULL) {
        printf("wrong format!Must be 'set environment key=value'\n");
    }
    printf("name:%s value:%s\n", name, tmp);
    char *old_value = getenv(name);
    strcpy(lctx->environ_list[lctx->env_num].name, name);
    if( old_value == NULL) {
        lctx->environ_list[lctx->env_num].value = NULL;
    } else {
        strcpy(lctx->environ_list[lctx->env_num].value, old_value);
    }
    lctx->env_num++;

    putenv((char*)cmdbuffer);

    return 0;
}

