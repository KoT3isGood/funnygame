#pragma once
#include "stdint.h"


typedef struct {
  uint32_t albedo;
  uint32_t normal;
} material;


typedef struct {
  char* data;
  uint32_t numindicies;
  uint32_t numverices;
  uint32_t numnormals;
  uint32_t numuvs;
  uint32_t nummaterials;
  float* vertex;
  uint32_t* index;
  float* normals;
  uint32_t* indexnormals;
  float* uvs;
  uint32_t* indexuvs;
  material* materials;
} modelinfo;


void fuck(const char* why);
char* strclone(const char *format, ...);
modelinfo readmodel(const char* file);
void print_hex(const unsigned char *data, uint64_t length);
