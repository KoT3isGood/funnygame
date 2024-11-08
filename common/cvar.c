#include "cvar.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

cvar_t* vars;

cvar_t* cvar_findvar(const char* name) {
  cvar_t* var;
  for(var = vars; var; var=(cvar_t*)var->next) {
    if (!strcmp(var->name,name)) {
      return var;
    };
  };
  return 0;
};

cvar_t* cvar_get(const char* name, const char* value, int flags) {
  cvar_t* var = cvar_findvar(name);
  if (var) return var;
  cvar_t* newvar = (cvar_t*)malloc(sizeof(cvar_t));
  newvar->name = strclone(name);
  newvar->value = strclone(value);
  newvar->flags = flags;
  newvar->next = vars;
  vars=newvar;
  return newvar;
};
cvar_t* cvar_fget(const char* name, float value, int flags) {
  cvar_t* var = cvar_findvar(name);
  if (var) return var;
  cvar_t* newvar = (cvar_t*)malloc(sizeof(cvar_t));
  newvar->name = strclone(name);
  newvar->value = strclone("%f",value);
  newvar->flags = flags;
  newvar->next = vars;
  vars=newvar;
  return newvar;
};

void cvar_setvalue(cvar_t* cvar, const char* value) {
  if (cvar->flags&CVAR_READONLY||cvar->flags&CVAR_PROTECTED) return;
  free(cvar->value);
  cvar->value = strclone(value);
};
void cvar_setfvalue(cvar_t* cvar, float value) {
  if (cvar->flags&CVAR_READONLY||cvar->flags&CVAR_PROTECTED) return;
  free(cvar->value);
  cvar->value = strclone("%s",value);
};

const char* cvar_getvalue(cvar_t* cvar) {
  return cvar->value;
};
float cvar_getfvalue(cvar_t* cvar) {
  return atof(cvar->value);
};

