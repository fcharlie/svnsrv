//
//
//
//
#include <stdio.h>
#include "GetOptInc.h"

int main(int argc, char **argv) {
  const char *short_opt = "?Dc:dhvqs:";
  int ch = 0;
  int opt_index = 0;
  const char *cfile = nullptr;
  const char *signame = nullptr;
  bool daemon = false;
  bool debug = false;
  bool version = false;
  bool quiet = false;
  const option longopts[] = {{"daemon", no_argument, NULL, 'D'},
                             {"config", required_argument, NULL, 'c'},
                             {"debug", no_argument, NULL, 'd'},
                             {"help", no_argument, NULL, 'h'},
                             {"version", no_argument, NULL, 'v'},
                             {"quiet", no_argument, NULL, 'q'},
                             {"signal", required_argument, NULL, 's'},
                             {NULL, 0, 0, 0}};
  while ((ch = getopt_long(argc, argv, short_opt, longopts, &opt_index)) !=
         -1) {
    switch (ch) {
    case '?':
    case 'h':
      printf("Help \n");
      return 0;
    case 'D':
      daemon = true;
      break;
    case 'c':
      cfile = optarg;
      break;
    case 'd':
      debug = true;
      break;
    case 'v':
      version = true;
      break;
    case 'q':
      quiet = true;
      break;
    case 's':
      signame = optarg;
      break;
    default:
      break;
    }
  }

  return 0;
}
