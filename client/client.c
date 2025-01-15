#include "client.h"
#include "render.h"
#include "window.h"
#include "stdio.h"
#include "../common/common.h"
#include "../common/module.h"
#include "stdlib.h"
#include "unistd.h"

window mainwindow;
modelinfo m;
model mdl;
void client_init() {
  module_t* kernel = module_fork("kernel");
  if (!kernel) {
    printf("\n");
    printf("Failed to fork kernel module!\n");
    printf("Game cannot run without kernel\n");
    printf("Please verify game installation\n");
    exit(1);
  }


  sys_initwindows();
  draw_init();

  mainwindow = sys_createwindow();
  sys_setwindowtitle(mainwindow,"funnygame");
  sys_setwindowsize(mainwindow,1,1,1280,720);

  m = readmodel("teapot.bmf");
  mdl = draw_genmodel(m);


  printf("client inited\n");
};
void client_frame() {
  sys_prerender();
  float matrix[16] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1,
  };
  draw_model(mdl,matrix);
  sys_render();
};
bool client_shouldrun() {
  return sys_windowsexists(mainwindow);
};
void client_deinit() {

};
