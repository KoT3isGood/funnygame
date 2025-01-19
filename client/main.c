#include "client.h"

int main() {
  client_init();
  while (client_shouldrun()) {
    client_frame();
  }
  client_deinit();
  return 0;
}
