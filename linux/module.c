#include "../common/module.h"
#include "stdlib.h"
#include "memory.h"
#include "../common/common.h"
#include <string.h>
#include "stdio.h"
#include <dlfcn.h>

struct metadata_t {
  char* author;
  unsigned int version;
  unsigned int numdependencies;
  char* dependencies;
} metadata_t;

module_t* module_fork(const char* name) {
  struct appinfo {
    void* handler;
  } appinfo;

  // get filepath to the shared object

  module_t* module = malloc(sizeof(module_t));

  module->name = strclone("%s",name);
  module->objectwd = strclone("%s/lib%s.so", module->name, module->name);

  // try to open handle
  void* handle = dlopen(module->objectwd,RTLD_LAZY);
  if (!handle) {
    printf("failed to open module: %s\n",module->name);
    free(module->name);
    free(module->objectwd);
    free(module);
    return 0;
  }
  void(*module_init)(struct appinfo) = dlsym(handle,"module_init");
  if (!module_init) {
    printf("%s: failed to find symbol: module_init\n",module->name);
    printf("look at manual for module_init\n");
    free(module->name);
    free(module->objectwd);
    free(module);
    dlclose(handle);
    return 0;
  }
  module_init(appinfo);

  // look for main
  int(*module_main)() = dlsym(handle,"main");
  if (!module_main) {
    printf("%s: failed to find symbol: main\n",module->name);
    free(module->name);
    free(module->objectwd);
    free(module);
    dlclose(handle);
    return 0;
  }

  // now run main
  int result = module_main();
  if (!result) {
    
  }


  return module;
};
void module_kill(module_t* module) {
};
