#pragma once

typedef struct module_t {
  struct module_t* next;
  const char* name;
  const char* objectwd;
} module_t;

module_t* module_fork(const char* name);
void module_kill(module_t* module);
