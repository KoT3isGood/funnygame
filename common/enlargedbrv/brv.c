#include "brv.h"
#include "stdlib.h"
#include "memory.h"
#include "stdint.h"
#include "stdio.h"
#include "float.h"
#include "math.h"

lbrv_group_t lbrv_read(unsigned char* contents) {

  lbrv_group_t vehicle = {};
  vehicle.version = contents[0];
  vehicle.numobjects = contents[1]|contents[2]<<8;

  lbrv_object_t* startingbrick = 0;
  lbrv_object_t* currentbrick = 0;
  uint64_t p = 0;

  vehicle.numclasses = contents[3]|contents[4]<<8;
  vehicle.classes = (char**)malloc(sizeof(char*)*vehicle.numclasses);
  vehicle.numproperties = contents[5]|contents[6]<<8;
  p=7;
  
  // reads string size, the string and puts evertyhing into brickname
  for (int i = 0;i<vehicle.numclasses;i++) {
    char bricklen = contents[p++];
    char* brickname = (char*)malloc(bricklen+1);
    brickname[bricklen]=0;
    memcpy(brickname,contents+p,bricklen);
    vehicle.classes[i]=brickname;
    p+=bricklen;
  }  
  vehicle.parameters = malloc(sizeof(lbrv_parameter_t)*vehicle.numproperties);

  // reads properties
  // since each property element is unknown size we need to get offsets
  for (int i = 0;i<vehicle.numproperties;i++) {

    // read name
    lbrv_parameter_t parameter;
    char bricklen = contents[p++];
    char* brickname = (char*)malloc(bricklen+1);
    parameter.name = brickname;
    brickname[bricklen]=0;
    memcpy(brickname,contents+p,bricklen);
    p+=bricklen;


    // num elements
    uint16_t numelements=(contents[p]<<0)||(contents[p+1]<<8);
    p+=2;
    uint64_t datasize;
    memcpy(&datasize,&contents[p],sizeof(uint64_t));
    p+=8;
    parameter.data = malloc(datasize);
    memcpy(parameter.data,contents+p,datasize);
    p+=datasize;

    if(numelements>1) {

      uint64_t elementsize;
      memcpy(&elementsize,&contents[p],sizeof(uint64_t));
      p+=sizeof(uint64_t);
      parameter.size=0;


      if (elementsize>0) {
        int offset = 0;
        parameter.sizes=(uint64_t*)malloc(sizeof(uint64_t)*numelements);
        parameter.offsets=(uint64_t*)malloc(sizeof(uint64_t)*numelements);

        for (int i = 0;i<numelements;i++) {

          uint32_t size=elementsize;
          parameter.sizes[i]=size;
          parameter.offsets[i]=offset;
          offset+=size;
        }

      } else {
        int offset = 0;
        parameter.sizes=(uint64_t*)malloc(sizeof(uint64_t)*numelements);
        parameter.offsets=(uint64_t*)malloc(sizeof(uint64_t)*numelements);

        for (int i = 0;i<numelements;i++) {

          uint32_t size=(contents[p]<<0)|(contents[p+1]<<8);
          memcpy(&datasize,&contents[p],sizeof(uint64_t));
          parameter.sizes[i]=size;
          parameter.offsets[i]=offset;
          offset+=size;
          p+=sizeof(uint32_t);
        }
      }

    } else {
      parameter.size=datasize;
    }

    vehicle.parameters[i]=parameter;
  }

  // now read all the bricks
  for (int i = 0;i<vehicle.numobjects;i++) {
    lbrv_object_t* brick = (lbrv_object_t*)malloc(sizeof(lbrv_object_t));

    // read class name first
    unsigned short brickid = (contents[p]<<0)|(contents[p+1]<<8);
    brick->name=vehicle.classes[brickid];

    // now read all properties
    p+=2;
    uint32_t datasize = (contents[p]<<0)+(contents[p+1]<<8)+((unsigned int)contents[p+2]<<16)+((unsigned int)contents[p+3]<<24);
    p+=4;
    unsigned char numproperties = (contents[p]);
    p+=1;
    brick->numparameters=numproperties;
    brick->parameters = (lbrv_parameter_t*)malloc(sizeof(lbrv_parameter_t)*numproperties);

    for (int i = 0;i<numproperties;i++) {
      unsigned short property=(contents[p]<<0)+(contents[p+1]<<8);
      unsigned short index = (contents[p+2]<<0)+(contents[p+3]<<8);

      if (!vehicle.parameters[property].size) {
        int offset = vehicle.parameters[property].offsets[index];
        lbrv_parameter_t param;
        param.name = vehicle.parameters[property].name;
        param.datasize = vehicle.parameters[property].sizes[index];
        param.data = vehicle.parameters[property].data+offset;
        brick->parameters[i]=param;
      } else {
        lbrv_parameter_t param;
        param.name = vehicle.parameters[property].name;
        param.datasize = vehicle.parameters[property].size;
        param.data = vehicle.parameters[property].data+param.datasize*index;
        brick->parameters[i]=param;
      }

      p+=4;
    }

    // position and rotation
    unsigned int x1 = (contents[p]<<0)+(contents[p+1]<<8)+((unsigned int)contents[p+2]<<16)+((unsigned int)contents[p+3]<<24);
    float x = *(float*)&x1/100.0;
    p+=4;
    unsigned int y1 = (contents[p]<<0)+(contents[p+1]<<8)+((unsigned int)contents[p+2]<<16)+((unsigned int)contents[p+3]<<24);
    float y = *(float*)&y1/100.0;
    p+=4;
    unsigned int z1 = (contents[p]<<0)+(contents[p+1]<<8)+((unsigned int)contents[p+2]<<16)+((unsigned int)contents[p+3]<<24);
    float z = *(float*)&z1/100.0;
    p+=4;
    
    brick->position[0]=x;
    brick->position[1]=y;
    brick->position[2]=z;

    x1 = (contents[p]<<0)+(contents[p+1]<<8)+((unsigned int)contents[p+2]<<16)+((unsigned int)contents[p+3]<<24);
    x = *(float*)&x1;
    p+=4;
    y1 = (contents[p]<<0)+(contents[p+1]<<8)+((unsigned int)contents[p+2]<<16)+((unsigned int)contents[p+3]<<24);
    y = *(float*)&y1;
    p+=4;
    z1 = (contents[p]<<0)+(contents[p+1]<<8)+((unsigned int)contents[p+2]<<16)+((unsigned int)contents[p+3]<<24);
    z = *(float*)&z1;
    p+=4;
    brick->rotation[0]=x;
    brick->rotation[1]=y;
    brick->rotation[2]=z;
    brick->next = 0;

    // push into the stack
    if (currentbrick) {
      currentbrick->next = brick;
      currentbrick=currentbrick->next;
    } else {
      startingbrick = brick;
      currentbrick=brick;
    }
  }
  vehicle.objects = startingbrick;  
  return vehicle;
};
typedef struct lbrv_class {
  struct brv_class* next;
  int classid;
  char* name;
} lbrv_class;

typedef struct lbrv_parameter2_t {
  uint32_t numelements;
  uint64_t datasize;
  uint32_t size;
  char* name;
  char isparamsizeconstant;
  int propertyid;


  struct parameter {
    struct parameter* next;
    int paramid;
    lbrv_parameter_t* param;
  }* parameters;
  struct parameter* startingparameters;
  struct lbrv_parameter2_t* next;
} lbrv_parameter2_t;

typedef struct lbrv_object2_t {
  int classid;
  int numparameters;
  struct brick_parameter {
    short propertyid;
    short paramid;
  }* parameters;
  float rotation[3];
  float location[3];
  struct lbrv_object2_t* next;
} lbrv_object2_t;


void lbrv_build(lbrv_group_t vehicle, uint64_t* size, unsigned char** data) {

  lbrv_class* startingclass=0;
  lbrv_class* currentclass=0;
  int numclasses=0;
  lbrv_parameter2_t* startingparameters=0;
  lbrv_parameter2_t* currentparameters=0;
  lbrv_object2_t* startingbrick=0;
  lbrv_object2_t* currentbrick=0;
  unsigned int filesize = 3;
  if (!data) {
    // uhh, it should be there
    printf("data is 0\n");
    return;
  }
  int numparams=0;
  filesize+=4;
  for (lbrv_object_t* brick=vehicle.objects;brick;brick=brick->next) {
    lbrv_object2_t* genbrick=(lbrv_object2_t*)malloc(sizeof(lbrv_object2_t));
    memset(genbrick,0,sizeof(lbrv_object2_t));
    int foundid=0;
    char found = 0;
    // create non-exisiting classes
    for(lbrv_class* class = startingclass;class;class=class->next) {
      if (!strcmp(class->name,brick->name)) {
        found=1;
        break;
      }
      foundid++;
    }

    if (found) {
    } else {
      lbrv_class* class = (lbrv_class*)malloc(sizeof(lbrv_class));
      class->name = (char*)malloc(strlen(brick->name)+1);
      class->next = 0;
      strcpy(class->name,brick->name);
      strcpy(class->name,class->name);
      if (currentclass) {
        currentclass->next = class;
        currentclass=currentclass->next;
      } else {
        startingclass = class;
        currentclass=class;
      }


      filesize+=strlen(brick->name)+1;
      numclasses++;
    }
    genbrick->classid=foundid;
    genbrick->parameters = (struct brick_parameter*)malloc(4*brick->numparameters);
    genbrick->numparameters = brick->numparameters;

    // same to properties
    // but we also have to check the data
    
    // too bad, but ehh,could be worse
    struct brick_parameter* startingparameter=0;
    struct brick_parameter* currentparameter=0;
    for (int i = 0;i<brick->numparameters;i++) {
      foundid = 0;
      found = 0;
      lbrv_parameter2_t* param=0;
      for(lbrv_parameter2_t* parameter = startingparameters;parameter;parameter=parameter->next) {
         if (!strcmp(parameter->name,brick->parameters[i].name)) {
          found=1;
          param=parameter;
          break;
        }
      foundid++;

      }
      if (found) {
      } else {
        param = (lbrv_parameter2_t*)malloc(sizeof(lbrv_parameter2_t));
        memset(param,0,sizeof(lbrv_parameter2_t));
        param->name = (char*)malloc(strlen(brick->parameters[i].name)+1);
        strcpy(param->name,brick->parameters[i].name);

        param->datasize=0;
        param->numelements=0;


        if (currentparameters) {
          currentparameters->next = param;
          currentparameters=currentparameters->next;
        } else {
          startingparameters = param;
          currentparameters=param;
        }

        numparams++;


        // name size + 10
        filesize+=strlen(brick->parameters[i].name)+11;
      }

      genbrick->parameters[i].propertyid=foundid;

      foundid = 0;
      found = 0; 
      // now find parameters
      for(struct parameter* par = param->startingparameters;par;par=par->next) {
        if (
            brick->parameters[i].datasize
            !=
            par->param->datasize) {
          foundid++;
          continue;
        }
        if (!memcmp(par->param->data,brick->parameters[i].data,par->param->datasize)) {
          found = 1;
          break;
        }
        foundid++;
      }
      
      genbrick->parameters[i].paramid=foundid;
      if (found) {
      } else {
        struct parameter* startingparam=param->startingparameters;
        struct parameter* currentparam=param->parameters;


        struct parameter* par = (struct parameter*)malloc(sizeof(struct parameter));
        par->next=0;
        if (currentparam) {
          currentparam->next = par;
          currentparam=currentparam->next;
        } else {
          startingparam = par;
          currentparam=par;
        }


        par->param=&brick->parameters[i];
        param->datasize+=par->param->datasize;

        
     
        filesize+=brick->parameters[i].datasize;
        param->numelements++;
        param->startingparameters = startingparam;
        param->parameters=currentparam;
      }
    }


    brick->position[0]*=100;
    brick->position[1]*=100;
    brick->position[2]*=100;
    memcpy(genbrick->location,brick->position,12);
    memcpy(genbrick->rotation,brick->rotation,12);
    if (currentbrick) {
      currentbrick->next = genbrick;
      currentbrick=currentbrick->next;
    } else {
      startingbrick = genbrick;
      currentbrick=genbrick;
    }
    filesize+=31;
    filesize+=4*genbrick->numparameters;

  }
  for(lbrv_parameter2_t* parameter = startingparameters;parameter;parameter=parameter->next) {
      int size = 0;
      if (parameter->numelements!=1) {
        char canbeshorted=1;
        for (struct parameter* param=parameter->startingparameters;param;param=param->next) {
          if (size==0) {
            size=param->param->datasize;
            continue;
          }
          if (size!=param->param->datasize) {
            canbeshorted=0;
          };
        }
        if (canbeshorted) {
          filesize+=4;
          parameter->size=size;
        } else {
          filesize+=4+4*parameter->numelements;
        }

      }
          };
  // fixes vehicle corruption
  filesize=ceil(filesize/16.0f+2)*16;
  
  *data = (unsigned char*)malloc(filesize);
  
  // data generation
  unsigned char* d=*data;
  unsigned int p = 0;
  d[p++]=vehicle.version;
  unsigned short numobjects=0;
  unsigned short numclasses2=0;
  unsigned short numproperties=0;
  for (lbrv_class* brick=startingclass;brick;brick=brick->next) {
    numclasses2++;
  }
  for (lbrv_object_t* brick=vehicle.objects;brick;brick=brick->next) {
    numobjects++;
  }
  for (lbrv_parameter2_t* parameter=startingparameters;parameter;parameter=parameter->next) {
    numproperties++;
  }
  memcpy(&d[p],&numobjects,2);
  p+=2;
  memcpy(&d[p],&numclasses2,2);
  p+=2;
  memcpy(&d[p],&numproperties,2);
  p+=2;
  for (lbrv_class* brick=startingclass;brick;brick=brick->next) {
    d[p++]=(char)strlen(brick->name);
    memcpy(&d[p],brick->name,(char)strlen(brick->name));
    p+=strlen(brick->name);
  }

  for (lbrv_parameter2_t* parameter=startingparameters;parameter;parameter=parameter->next) {
    d[p++]=(char)strlen(parameter->name);
    memcpy(&d[p],parameter->name,(char)strlen(parameter->name));
    p+=strlen(parameter->name);
    memcpy(&d[p],&parameter->numelements,2);
    p+=2;
    memcpy(&d[p],&parameter->datasize,8);
    p+=8;

    // now data
    for (struct parameter* param=parameter->startingparameters;param;param=param->next) {
      memcpy(&d[p],param->param->data,param->param->datasize);
      p+=param->param->datasize;
    }
    // sizes last
    if (!parameter->isparamsizeconstant) {
      if (parameter->numelements<2) {

      } else {
      if (parameter->size) {
        memcpy(&d[p],&parameter->size,4);
        p+=4;
      } else { 
          memset(&d[p],0,4);
          p+=4;
          for (struct parameter* param=parameter->startingparameters;param;param=param->next) {
            memcpy(&d[p],&param->param->datasize,2);
            p+=4;
          }
        }
      }
    }
  }

  // now bricks

  for (lbrv_object2_t* brick=startingbrick;brick;brick=brick->next) {
    memcpy(&d[p], &brick->classid,2);
    p+=2;
    unsigned int memory = 25;
    memory+=brick->numparameters*4;

    memcpy(&d[p], &memory,4);
    p+=4;
    memcpy(&d[p++], &brick->numparameters,1);

    for (int i = 0;i<brick->numparameters;i++) {
      memcpy(&d[p], &brick->parameters[i].propertyid,2);
      p+=2;
      memcpy(&d[p], &brick->parameters[i].paramid,2);
      p+=2;
    }
    memcpy(&d[p], &brick->location,12);
    p+=12;
    memcpy(&d[p], &brick->rotation,12);
    p+=12;
  }
  *size = filesize;


  return;
};
