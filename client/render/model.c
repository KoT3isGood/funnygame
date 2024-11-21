#include "../render.h"
#include "memory.h"
extern VkInstance instance;

extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
extern VkQueue draw;
extern VkQueue present;

extern VkCommandPool cmdPool;
extern VkCommandBuffer cmd[2];
extern VkCommandBuffer stagingCommandBuffer;
extern VkCommandPool stagingCommandPool;

model draw_genmodel(modelinfo info) {
  vk_buffer vertices = vk_genbuffer(info.numverices*12, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  vk_buffer indices = vk_genbuffer(info.numindicies*12, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  void* buffer;
  vkMapMemory(device,vertices.memory,0,vertices.size,0,&buffer);
  memcpy(buffer,info.vertex,vertices.size);
  vkUnmapMemory(device,vertices.memory);
};
void draw_copymodel(model m);
void draw_destroymodel(model m);
void draw_model(model m);
void draw_skinned(model m, skeleton s, animdata a);

void draw_initmodels() {
  
}

void draw_rendermodels() {

};

