#include "../client/window.h"
#include "../common/common.h"
#include "X11/Xlib.h"
#include "X11/X.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <vulkan/vulkan_core.h>
#include "vulkan/vulkan_xlib.h"

Display* display;
int screen;
int windowcount;
VkSwapchainKHR* swapchains = 0;
uint32_t* imageindexes;
char canrender = 0;

VkSemaphore* graphicsSemaphore;
VkSemaphore* presentSemaphore;

extern VkInstance instance;
extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
window_t* windows=0;
Atom WM_DELETE_WINDOW;


typedef struct window_handle_t {
  char* name;
  int imageIndex;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  Window window;
  VkImage* images;
  VkImageView* imageviews;
  VkSemaphore graphicsSemaphore[2];
  VkSemaphore presentSemaphore[2];
  int x;
  int y;
  int width;
  int height;
} window_handle_t;

window_t* findwindow2(Window wind) {
  window_t* window = 0;
  for(window=windows;window;window=(window_t*)window->next) {
    if (wind == ((window_handle_t*)window->handle)->window) return window;
  }
  return 0;
};

void sys_initwindows() {
  // open x11 display
  display = XOpenDisplay(0);
  if (!display) {
    fuck("failed to open display");
  }
  screen = DefaultScreen(display);
  WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
};

window_handle_t createwindow() {
  windowcount++;

  window_handle_t handle = {};
  handle.window = XCreateSimpleWindow(display,RootWindow(display,screen),0,0,1280,720,1,0,0);
  handle.name=0;
  XSelectInput(display,handle.window, StructureNotifyMask|KeyPressMask | StructureNotifyMask);
  
  XMapWindow(display,handle.window);
  XFlush(display);

  return handle;
}
window_handle_t createswapchain(window_handle_t handle) {
  {
    VkXlibSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.window = handle.window;
    createInfo.dpy = display;
    VkResult r = vkCreateXlibSurfaceKHR(instance,&createInfo,0,&handle.surface);
    if (r) {
      printf("%i\n",r);
      fuck("failed to create surface\n");
    }
  }

  {
    VkSurfaceCapabilitiesKHR capabilies={};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, handle.surface, &capabilies);
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = handle.surface;
    createInfo.minImageCount = capabilies.minImageCount;
    
    uint32_t numformats = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,handle.surface,&numformats,0);
    VkSurfaceFormatKHR* formats = malloc(sizeof(VkSurfaceFormatKHR)*numformats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,handle.surface,&numformats,formats);
    createInfo.imageFormat = formats[0].format;
    free(formats);

    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = capabilies.minImageExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; 
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkResult r = vkCreateSwapchainKHR(device, &createInfo, 0, &handle.swapchain);
    if (r) {
      fuck("failed to create swapchain\n");
    }  

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device, handle.swapchain, &imageCount, 0);
    handle.images=malloc(sizeof(VkImage)*imageCount);
    handle.imageviews=malloc(sizeof(VkImageView)*imageCount);
    VkResult res = vkGetSwapchainImagesKHR(device, handle.swapchain, &imageCount, handle.images);
    for (uint32_t i = 0; i < 2; i++) {
      handle.imageviews[i]=vk_genimageview(handle.images[i],createInfo.imageFormat );
    } 
    VkSemaphoreCreateInfo semaphoreCreateInfo={};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (int i = 0;i<2;i++) {
    vkCreateSemaphore(device, &semaphoreCreateInfo, 0, &handle.graphicsSemaphore[i]);
    vkCreateSemaphore(device, &semaphoreCreateInfo, 0, &handle.presentSemaphore[i]);
    }
  }
  return handle;
}

window_handle_t destroyswapchain(window_handle_t handle) {        
  for (int i = 0;i<2;i++) {
    vkDestroyImageView(device,handle.imageviews[i],0);
  }
  free(handle.imageviews);
  vkDestroySwapchainKHR(device,handle.swapchain,0);
  vkDestroySurfaceKHR(instance,handle.surface,0);
  free(handle.images);
  handle.swapchain=0;
  handle.surface=0;
  return handle;
}

window_handle_t destroywindow(window_handle_t handle) {
  for (int i = 0;i<2;i++) {
    vkDestroySemaphore(device,handle.graphicsSemaphore[i],0);
    vkDestroySemaphore(device,handle.presentSemaphore[i],0);
    handle.graphicsSemaphore[i]=0;
    handle.presentSemaphore[i]=0;
  }
  XDestroyWindow(display,handle.window);
  handle.window=0;
  return handle;
}
void sys_render() {
  draw_flush();
};

void sys_prerender() {
  vkDeviceWaitIdle(device);
  XEvent ev;
  if (XPending(display)) {
    XNextEvent(display,&ev);
    window_t* window; 
    window_handle_t* w; 
    printf("event\n");
    switch(ev.type) {
      case ConfigureNotify:
        printf("resizing\n");
        ;
        XConfigureEvent xce = ev.xconfigure;
 
        window = findwindow2(ev.xdestroywindow.window);
        w=window->handle;

        *w=destroyswapchain(*w);
        *w=createswapchain(*w);
        w->width=xce.width;
        w->height=xce.height;

        break;
      // TODO: handle keyboard
      case KeyPress:
        printf("pressing something\n");
        break;
      // window is destroyed
      case ClientMessage:
        if ((Atom)ev.xclient.data.l[0] == WM_DELETE_WINDOW) {
          printf("deleting window with atom\n");
          window=findwindow2(ev.xany.window);
          goto deletetry;
        }
        break;
deletetry:
        w=window->handle;
        if (!window) {
          printf("failed to find window\n");
          return;
        };
        // since using references handle referencing
        if (window==windows) {
          windows = window->next;
          goto deletesuccess;
        }

        window_t* wind;
        for(wind=windows;wind;wind=(window_t*)wind->next) {
          if ((window_t*)wind->next==window) {
            wind->next = window->next;
            break;
          };
        };
        goto deletesuccess;

deletesuccess:
        windowcount--;
        window_handle_t* w=window->handle;
        *w=destroyswapchain(*w);
        *w=destroywindow(*w);


        break;

    }
  }  
  if (!swapchains) {
    free(swapchains);
    free(imageindexes);
    free(graphicsSemaphore);
    free(presentSemaphore);
  }
  swapchains = malloc(sizeof(VkSwapchainKHR)*windowcount);
  imageindexes = malloc(sizeof(uint32_t)*windowcount);
  graphicsSemaphore = malloc(sizeof(VkSemaphore)*windowcount);
  presentSemaphore = malloc(sizeof(VkSemaphore)*windowcount);  
  int i = 0;
  for(window_t* window=windows;window;window=(window_t*)window->next) {
    window_handle_t* w=window->handle;
    swapchains[i]=w->swapchain;
    graphicsSemaphore[i]=w->graphicsSemaphore[w->imageIndex];
    presentSemaphore[i]=w->presentSemaphore[w->imageIndex];
    i+=1;
  }
  draw_sync();
  i=0;
  for(window_t* window=windows;window;window=(window_t*)window->next) {
    window_handle_t* w=(window_handle_t*)window->handle;
    w->imageIndex=imageindexes[i];
    i++;
  }
};

window sys_createwindow() {
  window_t* handle = malloc(sizeof(window_t));
  handle->handle=malloc(sizeof(window_handle_t));
  window_handle_t* w=(window_handle_t*)handle->handle;
  *w=createwindow();
  *w=createswapchain(*w);

  handle->next=windows;
  windows=handle;
  return handle;
};

void sys_setwindowsize(window wind, int x, int y, int width, int height) {
  if (!wind->handle) {
    fuck("invalid window\n");
    return;
  };
  window_handle_t* w=(window_handle_t*)wind->handle;
  w->width=width;
  w->height=height;
  XMoveResizeWindow(display, w->window, x,y,width,height);
  return; 
};

void sys_setwindowtitle(window wind, const char* title) {
  if (!wind->handle) {
    return;
  };
  window_handle_t* w=(window_handle_t*)wind->handle;
  w->name=strclone("%s",title);
  XStoreName(display,w->window,title);
};

void sys_destroywindow(window wind) {
  if (!wind->handle) {
    return;
  };
};
void sys_deinitwindows() {
  XCloseDisplay(display);
}

bool sys_windowsexists(window wind) {
  if (!wind->handle) {
    return false;
  };
  return true;
};

int sys_getwindowidth(window wind) {
  window_handle_t* w=(window_handle_t*)wind->handle;
  return w->width; 
};
int sys_getwindoheight(window wind) {
  window_handle_t* w=(window_handle_t*)wind->handle;
  return w->height; 
}


VkImage sys_getwindowimage(window wind) {
  window_handle_t* w=(window_handle_t*)wind->handle;
  return w->images[w->imageIndex]; 
};
VkImageView sys_getwindowimageview(window wind) {
  window_handle_t* w=(window_handle_t*)wind->handle;
  return w->imageviews[w->imageIndex];
};
