#pragma once
#include "vulkan/vulkan.h"
#include "../common/common.h"
#include <vulkan/vulkan_core.h>
#include "stdbool.h"


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

typedef struct vk_buffer {
  uint32_t size;
  VkBuffer buffer;
  VkDeviceMemory memory;
} vk_buffer;

typedef struct vk_tripipeline {
  VkPipeline pipeline;
  VkPipelineLayout layout;
  VkDescriptorSetLayout descriptorlayout;
  VkDescriptorSet descriptor;
} vk_tripipeline;

typedef struct vk_renderpass {
  VkFormat format;
  bool load;
  bool store;
} vk_renderpass;

typedef struct vk_tripipeline_info {
  // shaders our beloved
  // make sure to pass spirv
  uint32_t* types;
  uint32_t** shaders;
  uint32_t num;

  // descriptors and push constants
  int desciptorsnum;
  VkDescriptorSetLayoutBinding* descriptors;
  uint32_t pushsize;

  // inputs and outputs
  bool depth;
  int32_t renderpassnum;
  vk_renderpass* renderpass;
  // how we draw triangles
  VkPolygonMode polygonmode;
  int32_t linewidth;
  // other cool stuff
  bool msaa;
  
} vk_tripipeline_info;

typedef struct vk_blas {
  // blas info
  VkAccelerationStructureKHR accelerationstructure;
  vk_buffer asbuffer;
  vk_buffer scratchbuffer;

  // store other stuff, im to lazy to do as variables
  VkAccelerationStructureGeometryKHR asGeom;
	VkAccelerationStructureBuildRangeInfoKHR offset;
	VkAccelerationStructureBuildGeometryInfoKHR geometryInfo;
} vk_blas;

vk_buffer vk_genbuffer(uint32_t size,VkBufferUsageFlags usage);
vk_tripipeline vk_gentripipeline(vk_tripipeline_info info);



VkImageView vk_genimageview(const VkImage image, VkFormat format);

