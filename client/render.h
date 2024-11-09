#pragma once

typedef void* buffer;
typedef void* image;
typedef void* model;
typedef int shader;
typedef void* skeleton;
typedef void* animdata;

void draw_genmodel(buffer v, buffer i, buffer nv, buffer ni, buffer uv, buffer uvi, shader* s, int snum);
void draw_model(model m);
void draw_skinned(model m, skeleton s, animdata a);
void draw_triangles(buffer b);


void draw_init();
void draw_flush();
void draw_deinit();
