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
int windowcount;
VkSwapchainKHR* swapchains = 0;
uint32_t* imageindexes;
bool canrender = false;


typedef struct xwindow {
  struct xwindow* next;
  Window wind;
  char* name;
  int windowimage;
  VkSwapchainKHR swapchain;
  VkSurfaceKHR surface;
  VkImage images[2];
  VkImageView imageviews[2];
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

bool hasremade;
void sys_prerender() {
  XEvent ev;
  if (XPending(dp)) {
    XNextEvent(dp,&ev);
    switch(ev.type) {
      case DestroyNotify:
        ;
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
        windowcount--;
        vkDestroySwapchainKHR(device,window->swapchain,0);
        vkDestroySurfaceKHR(instance,window->surface,0);
        free(window);

        break;
      case KeyPress:
        printf("pressing something\n");
        break;
    }
  }
  if (!swapchains) {
    free(swapchains);
    free(imageindexes);
  }
  swapchains = (VkSwapchainKHR*)malloc(sizeof(VkSwapchainKHR)*windowcount);
  imageindexes = (uint32_t*)malloc(sizeof(uint32_t)*windowcount);
  int i = 0;
  for(xwindow* window=windows;window;window=(xwindow*)window->next) {
    swapchains[i]=window->swapchain;
    i+=1;
  }
  if (!windowcount) return;
  hasremade=false;
rerender:
  canrender = true;
  int status = draw_sync();
  if (status) {
    canrender = false;
    if (status==1000001003) {
      hasremade=true;
      // resize
      for(window* wind=windows;wind;wind=((xwindow*)wind)->next) {
        ;
        xwindow* window = wind;
        vkDestroySwapchainKHR(device,window->swapchain,0);
        vkDestroySurfaceKHR(instance,window->surface,0);
        {
          VkXlibSurfaceCreateInfoKHR createInfo = {};
          createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
          createInfo.window = window->wind;
          createInfo.dpy = dp;
          VkResult r = vkCreateXlibSurfaceKHR(instance,&createInfo,0,&window->surface);
          if (r) {
            printf("%i\n",r);
            fuck("failed to create surface\n");
          }
        }

        {
          VkSurfaceCapabilitiesKHR capabilies={};
          vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, window->surface, &capabilies);
          VkSwapchainCreateInfoKHR createInfo = {};
          createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
          createInfo.surface = window->surface;
          createInfo.minImageCount = 2;
          
          uint32_t numformats = 0;
          vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,window->surface,&numformats,0);
          VkSurfaceFormatKHR* formats = malloc(sizeof(VkSurfaceFormatKHR)*numformats);
          vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,window->surface,&numformats,formats);
          createInfo.imageFormat = formats[0].format;
          free(formats);

          createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
          createInfo.imageExtent = capabilies.minImageExtent;
          createInfo.imageArrayLayers = 1;
          createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
          createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

          createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; 
          createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
          VkResult r = vkCreateSwapchainKHR(device, &createInfo, 0, &window->swapchain);
          if (r) {
            fuck("failed to create swapchain\n");
          }  

          uint32_t imageCount = 2;
          vkGetSwapchainImagesKHR(device, window->swapchain, &imageCount, window->images);
          for (uint32_t i = 0; i < 2; i++) {
            window->imageviews[i]=vk_genimageview(window->images[i],createInfo.imageFormat );
          }
        }

      };
      printf("resizing done\n");
      goto rerender;
    }
    printf("status: %i\n",status);
    return;
  };
  i=0;
  for(xwindow* window=windows;window;window=(xwindow*)window->next) {
    window->windowimage=imageindexes[i];
    i++;
  }
};
void sys_render() { 
  if (canrender) {
    draw_flush();
  }
};
void sys_deinitwindows() {
  XCloseDisplay(dp);
};

window sys_createwindow() {
  xwindow* w = (xwindow*)calloc(1,sizeof(xwindow));

  w->wind = XCreateSimpleWindow(dp,RootWindow(dp,screen),0,0,1280,720,1,0,0);
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

    uint32_t numformats = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,w->surface,&numformats,0);
    VkSurfaceFormatKHR* formats = malloc(sizeof(VkSurfaceFormatKHR)*numformats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,w->surface,&numformats,formats);
    createInfo.imageFormat = formats[0].format;
    free(formats);

    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = capabilies.minImageExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; 
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkCreateSwapchainKHR(device, &createInfo, 0, &w->swapchain);  

    uint32_t imageCount = 2;
	  vkGetSwapchainImagesKHR(device, w->swapchain, &imageCount, w->images);
    for (uint32_t i = 0; i < 2; i++) {
      w->imageviews[i]=vk_genimageview(w->images[i],createInfo.imageFormat );
    }
  }
  windowcount++;

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

VkImage sys_getwindowimage(window wind) {
  return (
      (xwindow*)wind
      )->images[
    ((xwindow*)wind)->windowimage
      ];
};
VkImageView sys_getwindowimageview(window wind) {
  return ((xwindow*)wind)->imageviews[((xwindow*)wind)->windowimage];
};

bool sys_windowsexists(window wind) {
  return findwindow2(((xwindow*)wind)->wind)>0;
};
