#include "common.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "module.h"


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
void print_hex(const unsigned char *data, long length) {
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

void common_init() {
  cmd_init();

  module_t* kernel = module_fork("kernel");
  if (!kernel) {
    printf("\n");
    printf("Failed to fork kernel module!\n");
    printf("Game cannot run without kernel\n");
    printf("Please verify game installation\n");
    exit(1);
  }
};
void common_deinit() {

};
