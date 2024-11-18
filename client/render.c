#include "render.h"
#include "vulkan/vulkan.h"
#include "stdio.h"
#include <vulkan/vulkan_core.h>
#include "X11/Xlib.h"
#include "vulkan/vulkan_xlib.h"
#include "../common/common.h"
#include "stdlib.h"
#include "window.h"

model draw_genmodel(modelinfo info);
void draw_copymodel(model m);
void draw_destroymodel(model m);
void draw_model(model m);
void draw_skinned(model m, skeleton s, animdata a);

VkInstance instance;

VkPhysicalDevice physicalDevice;
VkDevice device;
VkQueue draw;
VkQueue present;

VkCommandPool cmdPool;
VkCommandBuffer cmd[2];
VkCommandBuffer stagingCommandBuffer;
VkCommandPool stagingCommandPool;

VkFence fence[2];
VkSemaphore graphicsSemaphore[16];
VkSemaphore presentSemaphore[16];

int imageIndex;


void draw_init() { 
  {
    printf("drawing\n");
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
    VkCommandPoolCreateInfo cmdPoolCreateInfo={};
    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolCreateInfo.queueFamilyIndex = 0;
    vkCreateCommandPool(device, &cmdPoolCreateInfo, 0, &cmdPool);

    VkCommandBufferAllocateInfo cmdAllocateInfo={};
    cmdAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocateInfo.commandBufferCount = 2;
    cmdAllocateInfo.commandPool = cmdPool;
    vkAllocateCommandBuffers(device, &cmdAllocateInfo, cmd);

    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolCreateInfo.queueFamilyIndex = 0;
    vkCreateCommandPool(device, &cmdPoolCreateInfo, 0, &stagingCommandPool);

    cmdAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocateInfo.commandBufferCount = 1;
    cmdAllocateInfo.commandPool = stagingCommandPool;
    vkAllocateCommandBuffers(device, &cmdAllocateInfo, &stagingCommandBuffer);
  }
  {
    VkFenceCreateInfo fenceCreateInfo={};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo={};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (uint32_t i = 0; i < 2;i++) {
      vkCreateFence(device, &fenceCreateInfo, 0, &fence[i]);
    }
    for (uint32_t i = 0; i < 16;i++) {
      vkCreateSemaphore(device, &semaphoreCreateInfo, 0, &graphicsSemaphore[i]);
      vkCreateSemaphore(device, &semaphoreCreateInfo, 0, &presentSemaphore[i]);
    }
  }
  printf("created drawer\n");
};

extern int windowcount;
extern VkSwapchainKHR* swapchains;
extern uint32_t* imageindexes;


int draw_sync() {
  vkWaitForFences(device,1,&fence[imageIndex],VK_TRUE,UINT64_MAX);
  vkResetFences(device,1,&fence[imageIndex]);

  if (!windowcount) {
    return 1;
  }
  for (int i = 0; i<windowcount;i++) {
    VkResult r = vkAcquireNextImageKHR(device, swapchains[i], UINT64_MAX,presentSemaphore[i], 0, &imageindexes[i]);
    if (r!=VK_SUCCESS) {
      return 1;
    };
  }
  return 0;
};

extern window mainwindow;
void draw_flush() {
  if (!windowcount) {
    return;
  }

  vkResetCommandBuffer(cmd[imageIndex],0);
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd[imageIndex],&beginInfo);
  VkImageSubresourceRange isr = {};
  isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  isr.layerCount=1;
  isr.levelCount=1;
  VkClearColorValue ccv = {
    1,0,0,0
  };
  vkCmdClearColorImage(cmd[imageIndex],sys_getwindowimage(mainwindow), VK_IMAGE_LAYOUT_GENERAL,&ccv,1,&isr);
  vkEndCommandBuffer(cmd[imageIndex]);
  VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreCount = windowcount;
	submitInfo.pWaitSemaphores = presentSemaphore;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd[imageIndex];

	submitInfo.signalSemaphoreCount = windowcount;
	submitInfo.pSignalSemaphores = graphicsSemaphore;

	vkQueueSubmit(draw, 1, &submitInfo, fence[imageIndex]);

	VkResult* results = (VkResult*)malloc(sizeof(VkResult)*windowcount);
	VkPresentInfoKHR presentInfo={};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = windowcount;
	presentInfo.pWaitSemaphores = graphicsSemaphore;
	presentInfo.swapchainCount = windowcount;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = imageindexes;
	presentInfo.pResults = results;
	VkResult r = vkQueuePresentKHR(present, &presentInfo);
};
void draw_deinit() {
  vkDestroyDevice(device,0);
  vkDestroyInstance(instance,0);
};




VkImageView vk_genimageview(const VkImage image, VkFormat format) {	
	VkImageView imageView = 0;
	VkImageViewCreateInfo imageViewCreateInfo={};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	vkCreateImageView(device, &imageViewCreateInfo, 0, &imageView);
	return imageView;
}
