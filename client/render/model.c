#include "../render.h"
#include "stdlib.h"
#include "memory.h"
#include <vulkan/vulkan_core.h>
#include "stdio.h"
extern VkInstance instance;

extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
extern VkQueue draw;
extern VkQueue present;

extern VkCommandPool cmdPool;
extern VkCommandBuffer cmd[2];
extern VkCommandBuffer stagingCommandBuffer;
extern VkCommandPool stagingCommandPool;

typedef struct vk_model {
  vk_buffer vertices;
  vk_buffer indicies;
  float matrix[16];
} vk_model;

typedef struct vk_staticmesh {
  vk_model* model;
  struct mesh {
    struct mesh* next;
    float matrix[16];
  }* meshes;
  struct vk_staticmesh* next;
} vk_staticmesh;
vk_staticmesh* meshes=0;


model draw_genmodel(modelinfo info) {
  vk_buffer vertices = vk_genbuffer(info.numverices*12, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  vk_buffer indices = vk_genbuffer(info.numindicies*12, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  void* buffer;
  VkResult r = vkMapMemory(device,vertices.memory,0,vertices.size,0,&buffer);
  memcpy(buffer,info.vertex,vertices.size);
  vkUnmapMemory(device,vertices.memory);

  vkMapMemory(device,indices.memory,0,indices.size,0,&buffer);
  memcpy(buffer,info.index,indices.size);
  vkUnmapMemory(device,indices.memory);

  vk_model* mod = (vk_model*)malloc(sizeof(vk_model));
  mod->vertices = vertices;
  mod->indicies = indices;

  return (model)mod;
};
void draw_copymodel(model m);
void draw_destroymodel(model m);

int numdrawedmodels = 0;
int numuniquemodels = 0;
void draw_model(model m, float matrix[16]) {
  vk_staticmesh* foundmesh=0;
  for (vk_staticmesh* mesh = meshes;mesh;mesh=mesh->next) {
    if (mesh->model==m) {
      foundmesh=mesh->model;
      break;
    }
  };
  if (!foundmesh) {
    foundmesh = malloc(sizeof(vk_staticmesh));
    foundmesh->model = m;
    foundmesh->meshes=0;
    foundmesh->next = meshes;
    meshes = foundmesh;
    numuniquemodels++;
  }

  struct mesh* mesh = malloc(sizeof(struct mesh));
  memcpy(mesh->matrix,matrix,64);
  mesh->next = foundmesh->meshes;
  foundmesh->meshes = mesh;
  numdrawedmodels++;
};
void draw_skinned(model m, skeleton s, animdata a);

vk_buffer instances;
vk_buffer matrices;
void draw_initmodels() {
  memset(&instances,0,sizeof(vk_buffer));
  vk_shader shader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_COMPUTE_BIT,"transform");
  vk_shader vertshader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_VERTEX_BIT,"vertex");
  vk_shader fragshader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_FRAGMENT_BIT,"fragment");

  vk_tripipeline_info info = {};
  vk_shader shaders[2] = {vertshader,fragshader};
  info.numshaders=2;
  info.shaders = shaders;

  info.desciptorsnum=0;
  info.polygonmode=VK_POLYGON_MODE_FILL;
  vk_tripipeline pipeline = vk_gentripipeline(info);
}
void draw_rendermodels() { 

  if (instances.buffer!=0) {
    vk_freebuffer(instances);
  }
  memset(&instances,0,sizeof(vk_buffer));
  // there is nothing to render
  if (!numuniquemodels) {
    return;
  }
  if (!numdrawedmodels) {
    return;
  }

  // make buffers
  instances = vk_genbuffer(numuniquemodels*sizeof(VkDrawIndexedIndirectCommand),VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);


  // set data
  VkDrawIndexedIndirectCommand* meshestbd;
  vkMapMemory(device,instances.memory,0,instances.size,0,&meshestbd);

  for (int i = 0;i<numuniquemodels;i++) {
    meshestbd[i].indexCount=instances.size/12;
  }

  vkUnmapMemory(device,instances.memory);



  vk_staticmesh* nextmesh=0;
  for (vk_staticmesh* mesh = meshes;mesh;mesh=nextmesh) {
    nextmesh = mesh->next;
    struct mesh* next=0;
    for (struct mesh* m = mesh->meshes;m;m=next) {
      next = m->next;
      free(m);
    }
    free(mesh);
  };
  meshes = 0;
  numdrawedmodels=0;
  numuniquemodels=0;
};

