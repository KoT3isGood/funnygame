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
