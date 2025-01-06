#pragma once
extern int cmd_argc;
extern char* cmd_argv[16];

typedef void(*cmdfunc_t)(void);

typedef struct cmdfunction_t {
  struct cmdfunction_t* next;
  const char* name;
  cmdfunc_t function; 
} cmdfunction_t;

void cmd_create(const char* name, cmdfunc_t function);
void cmd_execute(const char* command);
void cmd_execute2(const char* command);
