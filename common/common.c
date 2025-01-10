#include "common.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "math.h"

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
void print_hex(const unsigned char *data, size_t length) {
    size_t offset = 0;

    while (offset < length) {
        printf("%08zx  ", offset); // Print offset in hex
        size_t line_end = offset + 16; // Process up to 16 bytes per line

        // Print hex bytes
        for (size_t i = offset; i < line_end; i++) {
            if (i < length) {
                printf("%02x ", data[i]);
            } else {
                printf("   "); // Pad for alignment
            }
        }

        // Print ASCII representation
        printf(" ");
        for (size_t i = offset; i < line_end && i < length; i++) {
            printf("%c", (data[i] >= 32 && data[i] <= 126) ? data[i] : '.');
        }

        printf("\n");
        offset = line_end;
    }
}
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
  char* data = (char*)malloc(size);
  fread(data, sizeof(char), size, f);

  model.numindicies = *(uint32_t*)(data);
  model.numverices = *(uint32_t*)(data+4);
  model.numnormals = *(uint32_t*)(data+8);
  model.numuvs = *(uint32_t*)(data+12);
  model.index = data+16;
  model.indexuvs = ((char*)model.index     +model.numindicies*12);
  model.vertex = ((char*)model.indexuvs     +model.numindicies*12);
  model.uvs = ((char*)model.vertex          +model.numverices*12);
  model.data = data; 

  return model;
};
