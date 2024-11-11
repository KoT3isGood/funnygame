#include "client.h"
#include "window.h"
#include "stdio.h"

window mainwindow;
void client_init() {
  sys_initwindows();
  draw_init();

  mainwindow = sys_createwindow();
  sys_setwindowtitle(mainwindow,"funnygame");
  sys_setwindowsize(mainwindow,1,1,1280,720);

  window test2 = sys_createwindow();
  sys_setwindowtitle(test2,"help");
  window test3 = sys_createwindow();
  sys_setwindowtitle(test3,"help123");

  printf("client inited\n");
};
void client_frame() {
  sys_prerender();
};
bool client_shouldrun() {
  return sys_windowsexists(mainwindow);
};
void client_deinit() {

};
