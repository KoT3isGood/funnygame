#include "client.h"
#include "window.h"
#include "stdio.h"

void client_init() {
  sys_initwindows();
  window mainwind = sys_createwindow();
  sys_setwindowtitle(mainwind,"funnygame");
  sys_setwindowsize(mainwind,1,1,1280,720);
  printf("client inited\n");
};
void client_frame() {
  sys_prerender();
};
void client_deinit() {

};
