#pragma once
#include "common.h"

typedef enum {
  CVAR_READONLY = 0x01,   // is read only
  CVAR_PROTECTED = 0x01,   // is protected by the server
} cvar_flag;

typedef struct {
  struct cvar_t* next;
  const char* name;
  const char* value;
  int flags;
} cvar_t;


// returns cvar if exists
// creates and returns var if doesn't exist
cvar_t* cvar_get(const char* name, const char* value, int flags);
cvar_t* cvar_fget(const char* name, float value, int flags);


// sets cvar's value
void cvar_setvalue(cvar_t* cvar, const char* value);
void cvar_setfvalue(cvar_t* cvar, float value);

// gets cvar's value
const char* cvar_getvalue(cvar_t* cvar);
float cvar_getfvalue(cvar_t* cvar);
