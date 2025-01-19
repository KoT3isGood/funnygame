#include "server.h"
#include "stdio.h"
#include "stdlib.h"
void server_init() {
  common_init();
};
void server_frame() {
  char *buffer = NULL;
  size_t size = 0;
  printf("> ");
  ssize_t length = getline(&buffer, &size, stdin);
  if (length != -1) {
    cmd_execute(buffer);
  }
  free(buffer);
};
void server_deinit() {
  common_deinit();
};
char server_shouldrun() {
  return 1;
};

