/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#ifdef V7_EXE
int main(int argc, char *argv[]) {
  struct v7 *v7 = v7_create();
  int i;//, error_code;

  // Execute inline code
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      if (!v7_exec(v7, argv[i + 1])) {
        fprintf(stderr, "Error executing [%s]: %s\n", argv[i + 1],
                v7_get_error_string(v7));
      }
      i++;
    }
  }

  // Execute files
  for (; i < argc; i++) {
    if (!v7_exec_file(v7, argv[i])) {
      fprintf(stderr, "%s\n", v7_get_error_string(v7));
    }
  }

  v7_destroy(&v7);
  return EXIT_SUCCESS;
}
#endif
