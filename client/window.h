#pragma once
#include "stdbool.h"
#include "vulkan/vulkan.h"
#include "render.h"

// General window handler
typedef struct window_t {
  struct window_t* next;
  void* handle;
} window_t;
typedef window_t* window;

// Initializes window system
// eg: X11 or Win32
// should be called once
void sys_initwindows();

// Interacts with window system to interact with windows
// resize, close, input callbacks
void sys_prerender();

// Calls rendering API to render frames for all windows
void sys_render();

// Deinits window system
void sys_deinitwindows();



// TODO: rework windows
// issue: use after free, swapchain issues

// Creates basic window handler, 1280x720, no title
window sys_createwindow();

// Sets window's size and position
void sys_setwindowsize(window wind, int x, int y, int width, int height);

// Sets window's title
void sys_setwindowtitle(window wind, const char* title);

// Destroys window immediately
void sys_destroywindow(window wind);

// Returns true if window exists
bool sys_windowsexists(window wind);

int sys_getwindowidth(window wind);
int sys_getwindoheight(window wind);


// Vulkan rendering integration
// Should be used only during rendering
VkImage sys_getwindowimage(window wind);
VkImageView sys_getwindowimageview(window wind);
