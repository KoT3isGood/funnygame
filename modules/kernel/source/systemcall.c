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
  pid_t(*fork)(void);
  
} moduleinput_t;

metadata_t module_init(moduleinput_t input) {

};

int main() {

};
