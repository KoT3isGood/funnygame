#pragma once
#include "vulkan/vulkan.h"
#include "../common/common.h"
#include <vulkan/vulkan_core.h>
#include "vulkan/vk_enum_string_helper.h"
#include "stdbool.h"
#include "../common/model.h"


// Model handles
typedef void* model;
typedef void* skeleton;
typedef void* animdata;

// Creates model
// copies data from modelinfo to the GPU
model draw_genmodel(modelinfo_t* info);

// Creates another model copy
void draw_copymodel(model m);

// Destroys model
void draw_destroymodel(model m);

// Draws model
void draw_model(model m, float matrix[16]);

// Draws model with skinned data
void draw_skinned(model m, skeleton s, animdata a);

// Initializes vulkan
void draw_init();

// Syncronizes CPU and GPU
void draw_sync();

// Draws frame
void draw_flush();

// Deinits vulkan
void draw_deinit();


// Buffer handle
// be careful of use after free
typedef struct vk_buffer {
  uint32_t size;
  VkBuffer buffer;
  VkDeviceMemory memory;
} vk_buffer;

// Image handle
// be carefult of use after free
typedef struct vk_image {
  uint32_t x;
  uint32_t y;
  VkFormat format;
  VkImage image;
  VkDeviceMemory memory;
} vk_image;

// Rasterization pipeline handle 
// be careful of use after free
typedef struct vk_tripipeline {
  VkPipeline pipeline;
  VkPipelineLayout layout;
  VkDescriptorSet descriptor;
  VkRenderPass renderpass;
} vk_tripipeline;

// Render pass info
// use in vk_tripipeline_info
typedef struct vk_renderpass {
  VkFormat format;
  bool load;
  bool store;
} vk_renderpass;

// Shader handle
// be careful of use after free
typedef struct vk_shader {
  VkShaderStageFlagBits type;
  VkShaderModule module;
  VkPipelineShaderStageCreateInfo stageinfo;
} vk_shader;

// Descriptor set handle
typedef struct vk_descriptorset {
  VkDescriptorSetLayout layout;
} vk_descriptorset;

// Rasterization pipeline create info
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

// Compute pipeline create info
typedef struct vk_comppipeline_info {
  // takes only one shader
  vk_shader shader;
  // descriptors and push constants
  int desciptorsnum;
  VkDescriptorSetLayout* descriptors;
  uint32_t pushsize;
} vk_comppipeline_info;

// Bottom level acceleration structure handle
// requires VK_KHR_acceleration_structure extension
typedef struct vk_blas {
  // blas info
  VkAccelerationStructureKHR accelerationstructure;
  vk_buffer asbuffer;
  vk_buffer scratchbuffer;

  // Data storage
  VkAccelerationStructureGeometryKHR asGeom;
	VkAccelerationStructureBuildRangeInfoKHR offset;
	VkAccelerationStructureBuildGeometryInfoKHR geometryInfo;
} vk_blas;


// Creates buffer
vk_buffer vk_genbuffer(uint32_t size,VkBufferUsageFlags usage);
// Destroys buffer
void vk_freebuffer(vk_buffer buffer);

// Creates shader handle
// used in pipelines
vk_shader vk_genshader(const char* shaderfile, VkShaderStageFlagBits shadertype, const char* entry);

// Creates rasterization pipeline
vk_tripipeline vk_gentripipeline(vk_tripipeline_info info);

// Creates image
vk_image vk_genimage(unsigned int x, unsigned int y, VkFormat format);
// Destroys image
void vk_freeimage(vk_image image);

// Puts GPU barrier on image
// waits until pipeline stopped drawing
// or changes layout of the image
void vk_barrier(vk_image image, VkImageLayout layout);

// Generates image view
VkImageView vk_genimageview(const VkImage image, VkFormat format);

#define VK_PRINTRES(b,a) \
if (a<0) {printf("%s : %i (%s)\n", b, a, string_VkResult(a)); fuck("\n");} \
if (a>0) printf("%s : %i (%s)\n", b, a, string_VkResult(a))

