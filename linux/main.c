#include "../client/client.h"

int main(int argc, char** argv) {
  client_init();
  while (client_shouldrun()) {
    client_frame();
  }
  client_deinit();
  return 0;
}
