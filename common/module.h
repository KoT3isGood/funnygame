#pragma once
#include "../modules/shared.h"

typedef struct module_t {
  struct module_t* next;
  char* name;
  char* objectpath;
  void* object;
} module_t;

module_t* module_fork(const char* name);
void module_kill(module_t* module);
