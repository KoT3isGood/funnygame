#include "stdint.h"

typedef struct lbrv_parameter_t {
  char* name;
  uint64_t datasize;
  void* data;

  uint16_t numparameters;
  uint64_t size;
  uint64_t* sizes;
  uint64_t* offsets;
} lbrv_parameter_t;

typedef struct lbrv_object_t {
  struct lbrv_object_t* next;
  char* name;
  uint8_t numparameters;
  lbrv_parameter_t* parameters;

  float position[3];
  float rotation[3];
} lbrv_object_t;

typedef struct lbrv_group_t {
   uint8_t version;
   uint16_t numobjects; 
   uint16_t numclasses; 
   uint16_t numproperties;

  char** classes;
  lbrv_parameter_t* parameters;

  lbrv_object_t* objects;

} lbrv_group_t;

lbrv_group_t lbrv_read(unsigned char* contents);

void lbrv_close(lbrv_group_t vehicle);

void lbrv_build(lbrv_group_t vehicle, uint64_t* size, unsigned char** data);
