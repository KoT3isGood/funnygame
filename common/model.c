#include "common.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "model.h"


#define BRV_NO_DESERIALIZATION
#include "../includes/libbrv/include/brv.h"

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
  unsigned char* data = (char*)malloc(size);
  fread(data, sizeof(char), size, f);


  brv_vehicle modeldata = brv_read(data);
  for (brv_object* mesh = modeldata.bricks;mesh;mesh=mesh->next) {
    printf("%s\n",mesh->name);
    if (!strcmp(mesh->name,"Mesh")) {
      struct model_t* meshdata = malloc(sizeof(struct model_t));

      for (int i = 0; i<mesh->numparameters; i++) {
        brv_parameter parameter = mesh->parameters[i];
        printf("  %s %i\n",parameter.name,parameter.datasize);
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


