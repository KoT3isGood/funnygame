#pragma once
typedef void* window;

void sys_initwindows();
void sys_prerender();
void sys_render();
void sys_deinitwindows();

window sys_createwindow();
void sys_setwindowsize(window wind, int x1, int y1, int x2, int y2);
void sys_setwindowtitle(window wind, const char* title);
void sys_destroywindow(window wind);
