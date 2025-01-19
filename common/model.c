#include "common.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"


#define BRV_NO_DESERIALIZATION
#include "../includes/libbrv/include/brv.h"

modelinfo readmodel(const char* file) {
  modelinfo model;
 
  FILE* f = fopen(file,"rb");
  if (!f) {
    printf("failed to find file\n");
    exit(1);
  }
	uint32_t size = 0;
  fseek(f, 0, SEEK_END); // seek to end of file
  size = ftell(f); // get current file pointer
  fseek(f, 0, SEEK_SET); // seek back to beginning of file
  unsigned char* data = (char*)malloc(size);
  fread(data, sizeof(char), size, f);

  brv_vehicle modeldata = brv_read(data);
  for (brv_object* mesh = modeldata.bricks;mesh;mesh=mesh->next) {
    if (!strcmp(mesh->name,"Mesh")) {
      printf("mesh\n");
    }
  }
  exit(1);

  return model;
};


