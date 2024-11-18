#include "cmd.h"
#include "stdlib.h"
#include "string.h"
#include "common.h"
#include "stdio.h"
#include "stdbool.h"

int cmd_argc = 0;
char* cmd_argv[16];

cmdfunction_t* cmdfunctions = 0; 

cmdfunction_t* cmd_findcmd(const char* name) {
  cmdfunction_t* var;
  for(var = cmdfunctions; var; var=(cmdfunction_t*)var->next) {
    if (!strcmp(var->name,name)) {
      return var;
    };
  };
  return 0;
};

void cmd_create(const char* name, cmdfunc_t function) {
  cmdfunction_t* func = (cmdfunction_t*)malloc(sizeof(cmdfunction_t));
  func->name = strclone(name);
  func->function = function;
  func->next = cmdfunctions;
  cmdfunctions = func;
};
void cmd_execute2(const char* command) {
  // tokenize
  cmd_argc = 0;
  for (int i = 0;i<cmd_argc;i++) {
    if (cmd_argv[i]) free(cmd_argv[i]);
  }
  int i = 0;
  int count = 0;
  void* last = (void*)command;
  bool isquote = false;
  while(true) { 
    count++;
    if (command[i]=='"'&&!isquote) {
      last++;
      count--;
      isquote = true;
    } else 
    if ((command[i]==' ' && !isquote) || (command[i]=='"' && isquote) || command[i]=='\0') {
      if (count==1) {
        last++;
        count = 0;
        i++;
        if (command[i]=='\0') break;
        continue;
      };
      if (command[i]=='"') {
        isquote = false;
        //count--;
      }
      char* arg = (char*)malloc(count-1);
      memcpy(arg,last,count-1);
      cmd_argv[cmd_argc] = arg;
      cmd_argc++;
      last+=count;
      count = 0;
    }
    if (command[i]=='\0') {
      break;
    }
    i++; 
  }
  if (isquote) {
    printf("parser error\n");
    return;
  };
  if (cmd_argc==0) {
    return;
  }
  for (int i = 0;i<cmd_argc;i++) {
  }
  cmdfunction_t* func = cmd_findcmd(cmd_argv[0]);
  if (func) {
    func->function();
  } else {
    printf("invalid function\n");
  }
};
void cmd_execute(const char* command) {
  int i = 0;
  int count = 0;
  char* last = (char*)command;
  bool isquote = false;
  while(true) {
    count++;
    if (command[i]=='"') {
      isquote=!isquote;
    }
    if (command[i]=='\0'||(command[i]==';')&&isquote==false) {
      if (count<2) {
        last++;
        count = 0;
        i++;
        if (command[i]=='\0') break;
        continue;
      };
      char* arg = (char*)malloc(count-1);
      memcpy(arg,last,count-1);
      cmd_execute2(arg);
      free(arg);
      last+=count;
      count=0;
    };
    if (command[i]=='\0') break;
    i++;
  }
};
