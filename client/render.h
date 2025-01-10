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
void draw_model(model m, float matrix[16]);
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
  VkDescriptorSet descriptor;
  VkRenderPass renderpass;
} vk_tripipeline;

typedef struct vk_renderpass {
  VkFormat format;
  bool load;
  bool store;
} vk_renderpass;

typedef struct vk_shader {
  VkShaderStageFlagBits type;
  VkShaderModule module;
  VkPipelineShaderStageCreateInfo stageinfo;
} vk_shader;

typedef struct vk_descriptorset {
  VkDescriptorSetLayout layout;
} vk_descriptorset;

typedef struct vk_tripipeline_info {
  vk_shader* shaders;
  uint32_t numshaders;

  // descriptors and push constants
  int desciptorsnum;
  VkDescriptorSetLayout* descriptors;
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
void vk_freebuffer(vk_buffer buffer);

vk_shader vk_genshader(const char* shaderfile, VkShaderStageFlagBits shadertype, const char* entry);
vk_tripipeline vk_gentripipeline(vk_tripipeline_info info);



VkImageView vk_genimageview(const VkImage image, VkFormat format);

