#include "render.h"
#include "vulkan/vulkan.h"
#include "stdio.h"
#include <vulkan/vulkan_core.h>
#include "X11/Xlib.h"
#include "vulkan/vulkan_xlib.h"
#include "../common/common.h"
#include "stdlib.h"

void draw_genmodel(buffer v, buffer i, buffer nv, buffer ni, buffer uv, buffer uvi, shader* s, int snum);
void draw_model(model m);
void draw_skinned(model m, skeleton s, animdata a);
void draw_triangles(buffer b);

VkInstance instance;
VkPhysicalDevice physicalDevice;
VkDevice device;
VkQueue draw;
VkQueue present;

void draw_init() { 
  {
    // instance
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    const char* extensions[] = {
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    };
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkCreateInstance(&createInfo,0,&instance);
  }
  {
    // device
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance,&count,0);
    if (!count) {
      fuck("failed to find any GPUs");
    }
    VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice)*count);
    vkEnumeratePhysicalDevices(instance,&count,physicalDevices);
    for (int i = 0;i<count;i++) {
      VkPhysicalDeviceProperties features = {};
      vkGetPhysicalDeviceProperties(physicalDevices[0], &features);
      physicalDevice=physicalDevices[i];
      break;
    };
    free(physicalDevices);

    float priority = 1.0;
    VkDeviceQueueCreateInfo queueCreateInfo={};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &priority; 

    const char* extensions[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.enabledLayerCount = 0;
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    vkCreateDevice(physicalDevice,&createInfo, 0, &device);
    vkGetDeviceQueue(device,0,0,&draw);
    vkGetDeviceQueue(device,0,0,&present);
  }
  {
    // command buffers
  }
  printf("created drawer\n");
};
void draw_flush();
void draw_deinit() {
  vkDestroyDevice(device,0);
  vkDestroyInstance(instance,0);
};
