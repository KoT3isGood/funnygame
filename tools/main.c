#include "string.h"
#include "stdio.h"
#include "model.h"
#include "getopt.h"
void printhelp() {
  printf("--model %%arg -- converts obj to model\n");
  printf("--help        -- shows this menu\n");
};


int main(int argc, char** argv) {

    
    const struct option long_opts[] = {
        {"model", required_argument, NULL, 'm'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    int opt;
    int long_index = 0;
  while ((opt = getopt_long(argc, argv, "hm:", long_opts, &long_index)) != -1) {
    switch (opt) {
      case 'm':
        if (optarg) {
          objtobmf(optarg);
        }
        break;
      case 'h':
        printhelp();
        break;
      case '?':
        printf("unknown argument\n");
        printhelp();
        break;
      default:
        break;
    }
  }
}
