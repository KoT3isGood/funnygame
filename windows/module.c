#include "../common/module.h"
#include "stdlib.h"
#include "memory.h"
#include "../common/common.h"
#include <string.h>
#include "stdio.h"


module_t* module_fork(const char* name) {
  module_t* module = malloc(sizeof(module_t));
  module->name = strclone("%s",name);
  module->objectwd = strclone("%s/%s.dll", module->name, module->name);
  return module;
}
