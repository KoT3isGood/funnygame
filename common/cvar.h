#pragma once
#include "common.h"

typedef enum {
  CVAR_READONLY = 0x01,   // read only
  CVAR_PROTECTED = 0x02,   // protected by the server
  CVAR_RETURNVALID = 0x04
} cvar_flag;

typedef struct cvar_t {
  struct cvar_t* next;
  const char* name;
  const char* value;
  const char* description;
  int flags;
} cvar_t;


// returns cvar if exists
// creates and returns var if doesn't exist
cvar_t* cvar_get(const char* name, const char* value, int flags, const char* description);
cvar_t* cvar_fget(const char* name, float value, int flags, const char* description);


// sets cvar's value
void cvar_setvalue(cvar_t* cvar, const char* value);
void cvar_setfvalue(cvar_t* cvar, float value);

// gets cvar's value
const char* cvar_getvalue(cvar_t* cvar);
float cvar_getfvalue(cvar_t* cvar);
