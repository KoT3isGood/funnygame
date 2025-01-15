#include "unistd.h"

typedef struct metadata_t {
  char* author;
  unsigned int version;
  unsigned int numdependencies;
  char* dependencies;
  unsigned int functioncount;
  unsigned char** functionnames;
  void** functions; 
} metadata_t;

typedef struct moduleinput_t {
  void(*fork)(void);
  pid_t(*open)(const char*,int);
  
} moduleinput_t;

metadata_t module_init(moduleinput_t input) {

};

int main() {

};
