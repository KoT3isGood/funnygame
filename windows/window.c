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
bool canrender = false;


typedef struct xwindow {
  struct xwindow* next;
  HWND wind;
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
xwindow* findwindow2(HWND wind) {
  xwindow* window = 0;
  for(window=windows;window;window=(xwindow*)window->next) {
    if (wind == window->wind) return window;
  }
  return 0;
};

void sys_initwindows() {
  
};


extern VkInstance instance;
extern VkPhysicalDevice physicalDevice;
extern VkDevice device;

bool hasremade;
void sys_prerender() {
  

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
          VkWin32SurfaceCreateInfoKHR createInfo = {};
          createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
          createInfo.hwnd = window->wind;
          vkCreateWin32SurfaceKHR(instance,&createInfo,0,&window->surface);
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
          vkCreateSwapchainKHR(device, &createInfo, 0, &window->swapchain);  

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

};

LRESULT handlewindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

window sys_createwindow() {
  xwindow* w = (xwindow*)calloc(1,sizeof(xwindow));

  const char* CLASS_NAME  = "brWindow";

  WNDCLASS wc = { };

  wc.lpfnWndProc   = handlewindow;
  wc.lpszClassName = CLASS_NAME;

  RegisterClassA(&wc);

  w->wind = CreateWindowExA(
    0,                              // Optional window styles.
    CLASS_NAME,                     // Window class
    "Learn to Program Windows",    // Window text
    WS_OVERLAPPEDWINDOW,            // Window style

    // Size and position
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

    NULL,       // Parent window    
    NULL,       // Menu
    NULL,  // Instance handle
    NULL        // Additional application data
    );
  w->name = 0;
  w->next=windows;
  windows=w;
  if (w->wind == NULL)
  {
      fuck("failed to create window");
  }

  ShowWindow(w->wind, 1);
  UpdateWindow(w->wind);

  {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = w->wind;
    vkCreateWin32SurfaceKHR(instance,&createInfo,0,&w->surface);
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
  //XMoveResizeWindow(dp, ((xwindow*)wind)->wind, x1,y1,x2-x1,y2-y1);
};
void sys_setwindowtitle(window wind, const char* title) {
  if (((xwindow*)wind)->name) {
    free(((xwindow*)wind)->name);
  }
  ((xwindow*)wind)->name = strclone(title);
  //XStoreName(dp,((xwindow*)wind)->wind,title);
};
void sys_destroywindow(window wind) {
  //XDestroyWindow(dp,((xwindow*)wind)->wind);
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

LRESULT handlewindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

}
