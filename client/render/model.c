#include "../render.h"
#include "stdlib.h"
#include "memory.h"
#include <vulkan/vulkan_core.h>
extern VkInstance instance;

extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
extern VkQueue draw;
extern VkQueue present;

extern VkCommandPool cmdPool;
extern VkCommandBuffer cmd[2];
extern VkCommandBuffer stagingCommandBuffer;
extern VkCommandPool stagingCommandPool;

int numuniquemodels = 0;
typedef struct vk_model {
  uint32_t id;
  vk_buffer vertices;
  vk_buffer indicies;
  struct model_transfort {
    float matrix[12];
    struct model_transfort* next;
  }* transform;

  struct vk_model* next;
} vk_model;




model draw_genmodel(modelinfo info) {
  vk_buffer vertices = vk_genbuffer(info.numverices*12, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  vk_buffer indices = vk_genbuffer(info.numindicies*12, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  void* buffer;
  vkMapMemory(device,vertices.memory,0,vertices.size,0,&buffer);
  memcpy(buffer,info.vertex,vertices.size);
  vkUnmapMemory(device,vertices.memory);

  vkMapMemory(device,indices.memory,0,indices.size,0,&buffer);
  memcpy(buffer,info.vertex,indices.size);
  vkUnmapMemory(device,indices.memory);

  vk_model* mod = (vk_model*)malloc(sizeof(vk_model));
  mod->vertices = vertices;
  mod->indicies = indices;
  mod->id = numuniquemodels++;


  return (model)mod;
};
void draw_copymodel(model m);
void draw_destroymodel(model m);
void draw_model(model m) {

};
void draw_skinned(model m, skeleton s, animdata a);


void draw_initmodels() {
  
}

void draw_rendermodels() { 

  // there is nothing to render

  // make instances buffer
  //vk_buffer instances = vk_genbuffer(numuniquemodels*sizeof(VkDrawIndexedIndirectCommand),VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

  for (int i = 0;i<numuniquemodels;i++) {
    
  };

  //void* buffer;
  //vkMapMemory(device,instances.memory,0,instances.size,0,&buffer);
  //vkUnmapMemory(device,instances.memory);

  // reset
};

