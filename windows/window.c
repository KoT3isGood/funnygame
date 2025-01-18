#include "../client/window.h"
#include "../common/common.h"
#include "windows.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <vulkan/vulkan_core.h>
#include "vulkan/vulkan_win32.h"


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


typedef struct window_handle_t {
  char* name;
  int imageIndex;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  HWND window;
  VkImage* images;
  VkImageView* imageviews;
  VkSemaphore graphicsSemaphore[2];
  VkSemaphore presentSemaphore[2];
  int x;
  int y;
  int width;
  int height;
} window_handle_t;

window_t* findwindow2(HWND wind) {
  window_t* window = 0;
  for(window=windows;window;window=(window_t*)window->next) {
    if (wind == ((window_handle_t*)window->handle)->window) return window;
  }
  return 0;
};

void sys_initwindows() {
  
};

LRESULT handlewindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

window_handle_t createwindow() {
  windowcount++;

  window_handle_t handle = {};
  handle.name=0;

 const char* CLASS_NAME  = "brWindow";

  WNDCLASS wc = { };

  wc.lpfnWndProc   = handlewindow;
  wc.lpszClassName = CLASS_NAME;

  RegisterClassA(&wc);

  handle.window = CreateWindowExA(
    0,                              // Optional window styles.
    CLASS_NAME,                     // Window class
    "",    // Window text
    WS_OVERLAPPEDWINDOW,            // Window style

    // Size and position
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

    NULL,       // Parent window    
    NULL,       // Menu
    NULL,  // Instance handle
    NULL        // Additional application data
    );
  if (handle.window == NULL)
  {
      fuck("failed to create window");
  }

  ShowWindow(handle.window, 1);
  UpdateWindow(handle.window);

  return handle;
}
window_handle_t createswapchain(window_handle_t handle) {
  {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = handle.window;
    vkCreateWin32SurfaceKHR(instance,&createInfo,0,&handle.surface);  }

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
  handle.window=0;
  return handle;
}
void sys_render() {
  draw_flush();
};

void sys_prerender() {
  vkDeviceWaitIdle(device);
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
  SetWindowLongPtr(w->window, GWLP_USERDATA,(long long)w);

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
  return; 
};

void sys_setwindowtitle(window wind, const char* title) {
  if (!wind->handle) {
    return;
  };
  window_handle_t* w=(window_handle_t*)wind->handle;
  w->name=strclone("%s",title);
};

void sys_destroywindow(window wind) {
  if (!wind->handle) {
    return;
  };
};
void sys_deinitwindows() {
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

LRESULT handlewindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  window_t* window = (window_t*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  switch (uMsg) {
	case WM_DESTROY:
	

  }
}
