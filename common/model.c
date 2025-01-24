#include "common.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "model.h"
#include "enlargedbrv/brv.h"

typedef struct bmf_property_t {
  uint32_t datasize;
  uint8_t* data;
  uint64_t* sizes;
  uint64_t* offsets;
} bmf_property_t;

typedef struct bmf_object_t {
  uint16_t class;
  uint16_t numproperties;
} bmf_object_t;

modelinfo_t* readmodel(const char* file) {
  modelinfo_t* model = malloc(sizeof(modelinfo_t));
  model->nummodels=0;
  model->models=0;
 
  FILE* f = fopen(file,"rb");
  if (!f) {
    printf("failed to find file\n");
    exit(1);
  }
	uint32_t size = 0;
  fseek(f, 0, SEEK_END); // seek to end of file
  size = ftell(f); // get current file pointer
  fseek(f, 0, SEEK_SET); // seek back to beginning of file
  uint8_t* data = (uint8_t*)malloc(size);
  fread(data, sizeof(uint8_t), size, f);
  fclose(f);

  lbrv_group_t modeldata = lbrv_read(data);
  for (lbrv_object_t* mesh = modeldata.objects;mesh;mesh=mesh->next) {
    printf("%s\n",mesh->name);
    if (!strcmp(mesh->name,"Mesh")) {
      struct model_t* meshdata = malloc(sizeof(struct model_t));

      printf("%s\n",mesh->name);
      for (int i = 0; i<mesh->numparameters; i++) {
        lbrv_parameter_t parameter = mesh->parameters[i];
        printf(" c %s %llu\n",parameter.name,parameter.datasize);
        if (!strcmp(parameter.name,"Indices")) {
          meshdata->indexcount=parameter.datasize;
          meshdata->indexes=parameter.data;
        }
        if (!strcmp(parameter.name,"UVIndices")) {
          meshdata->indexcount=parameter.datasize;
          meshdata->uvindexes=parameter.data;
        }
        if (!strcmp(parameter.name,"Vertices")) {
          meshdata->vertexcount=parameter.datasize;
          meshdata->vertices=parameter.data;
        }
        if (!strcmp(parameter.name,"UVs")) {
          meshdata->uvcount=parameter.datasize;
          meshdata->uvs=parameter.data;
        }
      }

      model->nummodels++;
      meshdata->next=model->models;
      model->models=meshdata;
    }
  }
 
  return model;
};

void writemodel(unsigned char** data, uint32_t* datasize) {
  modelinfo_t* modelinfo;
};


