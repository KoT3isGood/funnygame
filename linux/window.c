#include "../client/window.h"
#include "X11/Xlib.h"
#include "X11/X.h"
#include "../common/common.h"
#include "stdlib.h"
#include "stdio.h"

Display* dp;
int screen;

typedef struct {
  Window wind;
} xwindow;

void sys_initwindows() {
  dp = XOpenDisplay(NULL);
  if (!dp) {
    fuck("failed to open display");
  }
  
  screen = DefaultScreen(dp);
};
void sys_prerender() {
  XEvent ev;
  XNextEvent(dp,&ev);
};
void sys_render() {
  
};
void sys_deinitwindows() {
  XCloseDisplay(dp);
};

window sys_createwindow() {
  xwindow* w = (xwindow*)malloc(sizeof(xwindow));
  w->wind = XCreateSimpleWindow(dp,RootWindow(dp,screen),0,0,1280,720,1,0,0);
  XSelectInput(dp,w->wind, ExposureMask | KeyPressMask);
  XMapWindow(dp,w->wind);
  XFlush(dp);
  return w;
};
void sys_setwindowsize(window wind, int x1, int y1, int x2, int y2) {
  XMoveResizeWindow(dp, ((xwindow*)wind)->wind, x1,y1,x2-x1,y2-y1);
};
void sys_setwindowtitle(window wind, const char* title) {
  XStoreName(dp,((xwindow*)wind)->wind,title);
};
void sys_destroywindow(window wind) {

};
