#include "../client/client.h"

int main(int argc, char** argv) {
  client_init();
  while (1) {
    client_frame();
  }
  client_deinit();
  return 0;
}
