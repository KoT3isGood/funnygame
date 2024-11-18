#include "client.h"
#include "render.h"
#include "window.h"
#include "stdio.h"

window mainwindow;
void client_init() {
  sys_initwindows();
  draw_init();

  mainwindow = sys_createwindow();
  sys_setwindowtitle(mainwindow,"funnygame");
  sys_setwindowsize(mainwindow,1,1,1280,720);

  printf("client inited\n");
};
void client_frame() {
  sys_prerender();
  sys_render();
};
bool client_shouldrun() {
  return sys_windowsexists(mainwindow);
};
void client_deinit() {

};
