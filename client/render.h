#pragma once
#include "vulkan/vulkan.h"

typedef void* buffer;
typedef void* image;

typedef struct {
  int albedo;
  int normal;
} material;
typedef struct {
  int numindicies;
  int numverices;
  int numnormals;
  int numuvs;
  int nummaterials;
  buffer vertex;
  buffer index;
  buffer normals;
  buffer indexnormals;
  buffer uvs;
  buffer indexuvs;
  material* materials;
} modelinfo;
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



VkImageView vk_genimageview(const VkImage image, VkFormat format);
