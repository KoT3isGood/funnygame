#include "common.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "math.h"

void fuck(const char* why) {
  printf("%s",why);
  exit(1);
};
char* strclone(const char *format, ...) {
    va_list args;
    va_list args_copy;
    va_start(args, format);

    // Copy the va_list to use it for size calculation
    va_copy(args_copy, args);

    // Estimate the required size for the formatted string
    int size = vsnprintf(NULL, 0, format, args_copy) + 1;

    // Allocate memory for the result string
    char *result = (char*)malloc(size);
    if (result != NULL) {
        vsnprintf(result, size, format, args);
    }

    va_end(args);
    va_end(args_copy);
    return result;
}
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
