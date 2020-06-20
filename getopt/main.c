

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "version.h"

#define NO_LETTER   (1)
#define INVALID_VALUE (0xFFFFFFFF)

typedef enum {
  reqargIndex = 0,
  optargIndex,
  noargIndex,
  noletterIndex,
  nullIndex
} option_index_t;

const struct option long_options[] =
{  
    {"reqarg", required_argument,NULL, 'r'},
    {"optarg", optional_argument,NULL, 'o'},
    {"noarg",  no_argument,         NULL,'n'},
    {"noletter", no_argument, NULL, NO_LETTER },
    {NULL,     0,                      NULL, NO_LETTER},
}; 

int a_val = INVALID_VALUE;
int b_val = INVALID_VALUE;
int c_val = INVALID_VALUE;
int d_val = INVALID_VALUE;
int r_val = INVALID_VALUE;
int o_val = INVALID_VALUE;
int n_val = INVALID_VALUE;
int no_letter = INVALID_VALUE;

// return 1: break, 0:continue
int setOption(int opt, char *optarg, char *argv[], int option_index) {
    switch(opt) {
      case 'a':
        if (optarg) {
          a_val = atoi(optarg);
        } else {
          a_val = 1;
        }
        break;
      case 'b':
        {
            b_val = atoi(optarg);
        }
        break;
      case 'c':
        c_val = atoi(optarg);
        break;
      case 'd':
        d_val = 1;
        if (optarg) {
          printf("warnning:option d no require argument. ignore value %s\n", optarg);
        }
        break;
      case 'r':
        {
            r_val = atoi(optarg);
        }
        break;
      case 'o':
        if (optarg) {
          o_val = atoi(optarg);
        } else {
          o_val = 1;
        }
        break;
      case 'n':
        n_val = 1;
        if (optarg) {
          printf("warnning:option n no require argument. ignore value %s\n", optarg);
        }
        break;
      case NO_LETTER:
        {
            switch(option_index) {
            case  noletterIndex:
              no_letter = 1;
              break;
            case  nullIndex:

            break;
            default:
            break;
            }
        }
        break;
      default:

        return 1;
    }

    return 0;
}

#define showV(x)  printf(#x"=%d\n", x)

void showValues() {
  printf("=========\n");
  showV(a_val);
  showV(b_val);
  showV(c_val);
  showV(d_val);
  showV(r_val);
  showV(o_val);
  showV(n_val);
  showV(no_letter);

  printf("\r\nbuild at %s by %s\r\n", BUILD_DATE, BUILD_LINUX_USER);
  printf("=========\n");

}

int main(int argc, char *argv[])
{
    int opt;
    int option_index = 0;
    char *string = "a::b:c:dr:o::n";
    
    //initValues();
    while((opt =getopt_long_only(argc,argv,string,long_options,&option_index))!= -1)
    {  
        printf("opt = %c\t\t", opt);
        printf("optarg = %s\t\t",optarg);
        printf("optind = %d\t\t",optind);
        printf("argv[optind] =%s\t\t", argv[optind]);
        printf("option_index = %d\n",option_index);

        if (setOption(opt, optarg, argv, option_index)) {
            showValues();
            return -1;
        }
    } 

    showValues();

    return 0; 
}

