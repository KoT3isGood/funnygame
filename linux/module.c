#include "../common/module.h"
#include "stdlib.h"
#include "memory.h"
#include "../common/common.h"
#include <string.h>
#include "stdio.h"
#include <dlfcn.h>





module_t* module_fork(const char* name) {
  // get filepath to the shared object
  appinfo_t appinfo = {};

  module_t* module = malloc(sizeof(module_t));

  module->name = strclone("%s",name);
  module->objectpath = strclone("%s/lib%s.so", module->name, module->name);

  // try to open handle
  void* handle = dlopen(module->objectpath,RTLD_LAZY);
  if (!handle) {
    printf("failed to open module: %s\n",module->name);
    free(module->name);
    free(module->objectpath);
    free(module);
    return 0;
  }
  void(*module_init)(appinfo_t) = dlsym(handle,"module_init");
  if (!module_init) {
    printf("%s: failed to find symbol: module_init\n",module->name);
    printf("look at manual for module_init\n");
    free(module->name);
    free(module->objectpath);
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
    free(module->objectpath);
    free(module);
    dlclose(handle);
    return 0;
  }

  // now run main
  int result = module_main();
  if (!result) {
    
  }


  module->object=handle;
  return module;
};
void module_kill(module_t* module) {
};
