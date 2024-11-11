#pragma once

typedef void* buffer;
typedef void* image;

typedef struct material {
  int albedo;
  int normal;
};
typedef struct {
  buffer vertex;
  buffer index;
  buffer normals;
  buffer indexnormals;
  buffer uvs;
  buffer indexuvs;
  material* materials;
  int nummaterials;
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
void draw_flush();
void draw_deinit();
