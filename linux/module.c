#include "../common/module.h"
#include "stdlib.h"
#include "memory.h"
#include "../common/common.h"
#include <string.h>
#include "stdio.h"
#include <dlfcn.h>

module_t* module_fork(const char* name) {
  struct appinfo {
    void* handler;
  } appinfo;

  // get filepath to the shared object

  int filepathsize = sizeof(name)*2+5;
  char* filepath = malloc(filepathsize);
  memset(filepath,0,filepathsize);

  module_t* module = malloc(sizeof(module_t));
  module->name = strclone("%s",module);
  strcat(filepath,module->name);
  strcat(filepath,"/");
  strcat(filepath,module->name);
  strcat(filepath,".so");
  module->objectwd=filepath;

  void* handle = dlopen(module->objectwd,RTLD_LAZY);
  if (!handle) {
    printf("failed to open module: %s\n",module->name);
    free(filepath);
    free(module);
    return 0;
  }
 
  // module_init is prebuilt
  void(*module_init)(struct appinfo) = dlsym(handle,"module_init");
  if (!module_init) {
    printf("%s: failed to find symbol: module_init\n",module->name);
    printf("it has to be defined as such:\n");
    printf("void module_init(struct appinfo)\n");
    free(filepath);
    free(module);
    dlclose(handle);
    return 0;
  }
  module_init(appinfo);

  int(*module_main)() = dlsym(handle,"main");
  if (!module_main) {
    printf("%s: failed to find symbol: main\n",module->name);
    printf("it has to be defined as such:\n");
    printf("int main()\n");
    free(filepath);
    free(module);
    dlclose(handle);
    return 0;
  }
  int result = module_main();


  return module;
};
void module_kill(module_t* module) {
  
};
