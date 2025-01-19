

typedef struct metadata_t {
  char* author;
  unsigned int version;
  unsigned int numdependencies;
  char* dependencies;
} metadata_t;


typedef struct appinfo_t {
  void* handler;
} appinfo_t;
