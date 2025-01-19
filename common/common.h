#pragma once
#include "stdint.h"
#include "cvar.h"
#include "cmd.h"




typedef struct modelinfo {
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
  }* models;

} modelinfo;


void fuck(const char* why);
char* strclone(const char *format, ...);
modelinfo readmodel(const char* file);
void print_hex(const unsigned char *data, long length);
double gettime();



void common_init();
void common_deinit();
