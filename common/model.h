#pragma once
#include "stdint.h"
typedef struct modelinfo_t {
  int nummodels;
  struct model_t {
    uint32_t indexcount;
    uint32_t vertexcount;
    uint32_t uvcount;
    uint32_t* uvindexes;
    uint32_t* indexes;
    float* vertices;
    float* uvs;
    unsigned char* data;
    struct model_t* next;
  }* models;

} modelinfo_t;
modelinfo_t* readmodel(const char* file);

