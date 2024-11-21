#include "common.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"

void fuck(const char* why) {
  printf("%s\n",why);
  int* a = 0;
  *a=1;
};
const char* strclone(const char* format, ...) {
  va_list args;
  va_start(args, format);
  int size = vsnprintf(0,0,format,args);
  size+=1;
  char* a = (char*)malloc(size);
  vsprintf(a, format, args);
  va_end(args);
  return a;
};

modelinfo readmodel(const char* file) {
  modelinfo model;
  /*
  model.numindicies = *(uint32_t*)(data);
  model.numverices = *(uint32_t*)(data+4);
  model.numnormals = *(uint32_t*)(data+8);
  model.numuvs = *(uint32_t*)(data+12);
  model.vertexIndex = data+16;
  model.normalIndex = ((char*)model->vertexIndex +model->indexCount*12);
  model.uvIndex = ((char*)model->vertexIndex     +model->indexCount*24);
  model.vertex = ((char*)model->vertexIndex      +model->indexCount*36);
  model.normal = ((char*)model->vertex+model->vertexCount*12);
  model.uv = ((char*)model->vertex+model->vertexCount*24);
  model.rawfile = data;
  */

  return model;
};
