#pragma once
extern int cmd_argc;
extern char* cmd_argv[16];

typedef void(*cmdfunc_t)(void);

typedef struct cmdfunction_t {
  struct cmdfunction_t* next;
  char* name;
  char* description;
  cmdfunc_t function; 
} cmdfunction_t;

cmdfunction_t* cmd_findcmd(const char* name);
// Creates command
void cmd_create(const char* name, cmdfunc_t function, const char* description);

// Executes full command
void cmd_execute(const char* command);

// Executes only single function
void cmd_execute2(const char* command);

void cmd_init();
