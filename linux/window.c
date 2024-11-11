#include "../client/window.h"
#include "../common/common.h"
#include "X11/Xlib.h"
#include "X11/X.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <vulkan/vulkan_core.h>
#include "vulkan/vulkan_xlib.h"

Display* dp;
int screen;


typedef struct {
  struct xwindow* next;
  Window wind;
  char* name;
  VkSwapchainKHR swapchain;
  VkSurfaceKHR surface;
} xwindow;
xwindow* windows=0;

xwindow* findwindow(const char* name) {
  xwindow* window = 0;
  for(window=windows;window;window=(xwindow*)window->next) {
    if (!strcmp(window->name,name)) return window;
  }
  return 0;
};
xwindow* findwindow2(Window wind) {
  xwindow* window = 0;
  for(window=windows;window;window=(xwindow*)window->next) {
    if (wind == window->wind) return window;
  }
  return 0;
};

void sys_initwindows() {
  dp = XOpenDisplay(NULL);
  if (!dp) {
    fuck("failed to open display");
  }
  
  screen = DefaultScreen(dp);
};


extern VkInstance instance;
extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
void sys_prerender() {
  XEvent ev;
  if (XPending(dp)) {
    XNextEvent(dp,&ev);
    switch(ev.type) {
      case DestroyNotify:
        printf("Event received for window ID: %llu\n", ev.xdestroywindow.window);
        printf("destroying window\n");

        xwindow* window = findwindow2(ev.xdestroywindow.window);
        if (!window) {
          printf("failed to find window\n");
          return;
        };
        if (window==windows) {
          windows = window->next;
          goto deletesuccess;
        }
        xwindow* wind;
        for(wind=windows;wind;wind=(xwindow*)wind->next) {
          if ((xwindow*)wind->next==window) {
            wind->next = window->next;
            break;
          };
        };

deletesuccess:
        vkDestroySwapchainKHR(device,window->swapchain,0);
        vkDestroySurfaceKHR(instance,window->surface,0);



        free(window);
        break;
      case KeyPress:
        printf("pressing something\n");
        break;
    }
  }
};
void sys_render() {
  
};
void sys_deinitwindows() {
  XCloseDisplay(dp);
};

window sys_createwindow() {
  xwindow* w = (xwindow*)malloc(sizeof(xwindow));
  w->wind = XCreateSimpleWindow(dp,RootWindow(dp,screen),0,0,1280,720,1,0,0);
  printf("%llu\n",w->wind);
  XSelectInput(dp,w->wind, KeyPressMask | StructureNotifyMask);
  w->name = 0;
  w->next=windows;
  windows=w;
  XMapWindow(dp,w->wind);
  XFlush(dp);

  {
    VkXlibSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.window = w->wind;
    createInfo.dpy = dp;
    vkCreateXlibSurfaceKHR(instance,&createInfo,0,&w->surface);
  }

  {
    VkSurfaceCapabilitiesKHR capabilies={};
	  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, w->surface, &capabilies);
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = w->surface;
    createInfo.minImageCount = 2;
#ifdef __linux__
    createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
#else 
    createInfo.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
#endif
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = capabilies.minImageExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; 
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkCreateSwapchainKHR(device, &createInfo, 0, &w->swapchain);  }

  return w;
};
void sys_setwindowsize(window wind, int x1, int y1, int x2, int y2) {
  XMoveResizeWindow(dp, ((xwindow*)wind)->wind, x1,y1,x2-x1,y2-y1);
};
void sys_setwindowtitle(window wind, const char* title) {
  if (((xwindow*)wind)->name) {
    free(((xwindow*)wind)->name);
  }
  ((xwindow*)wind)->name = strclone(title);
  XStoreName(dp,((xwindow*)wind)->wind,title);
};
void sys_destroywindow(window wind) {
  XDestroyWindow(dp,((xwindow*)wind)->wind);
};

bool sys_windowsexists(window wind) {
  return findwindow2(((xwindow*)wind)->wind)>0;
};
