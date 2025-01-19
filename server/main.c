#include "server.h"

int main() {
  server_init();
  while (server_shouldrun()) {
    server_frame();
  }
  server_deinit();
  return 0;
}
