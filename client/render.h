#pragma once
#include "vulkan/vulkan.h"
#include "../common/common.h"
#include <vulkan/vulkan_core.h>


typedef void* model;
typedef void* skeleton;
typedef void* animdata;

model draw_genmodel(modelinfo info);
void draw_copymodel(model m);
void draw_destroymodel(model m);
void draw_model(model m);
void draw_skinned(model m, skeleton s, animdata a);

void draw_init();
int draw_sync();
void draw_flush();
void draw_deinit();

typedef struct {
  uint32_t size;
  VkBuffer buffer;
  VkDeviceMemory memory;
} vk_buffer;

typedef struct {
  VkPipeline pipeline;
  VkPipelineLayout layout;
  VkDescriptorSetLayout descriptorlayout;
  VkDescriptorSet descriptor;
} vk_tripipeline;

typedef struct {

} vk_tripipeline_info;

vk_buffer vk_genbuffer(uint32_t size,VkBufferUsageFlags usage);
vk_tripipeline vk_gentripipeline(vk_tripipeline_info info);


VkImageView vk_genimageview(const VkImage image, VkFormat format);

