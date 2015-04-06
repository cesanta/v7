/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#if defined(_MSC_VER) && _MSC_VER >= 1800
#define fileno _fileno
#endif

#ifdef V7_EXE
#define V7_MAIN
#endif

#ifdef V7_MAIN

#include <sys/stat.h>

static void show_usage(char *argv[]) {
  fprintf(stderr, "V7 version %s (c) Cesanta Software, built on %s\n",
          V7_VERSION, __DATE__);
  fprintf(stderr, "Usage: %s [OPTIONS] js_file ...\n", argv[0]);
  fprintf(stderr, "%s\n", "OPTIONS:");
  fprintf(stderr, "%s\n", "  -e <expr>  execute expression");
  fprintf(stderr, "%s\n", "  -t         dump generated text AST");
  fprintf(stderr, "%s\n", "  -b         dump generated binary AST");
  exit(EXIT_FAILURE);
}

static char *read_file(const char *path, size_t *size) {
  FILE *fp;
  struct stat st;
  char *data = NULL;
  if ((fp = fopen(path, "rb")) != NULL && !fstat(fileno(fp), &st)) {
    *size = st.st_size;
    data = (char *) malloc(*size + 1);
    if (data != NULL) {
      if (fread(data, 1, *size, fp) != *size) {
        free(data);
        return NULL;
      }
      data[*size] = '\0';
    }
    fclose(fp);
  }
  return data;
}

static void print_error(struct v7 *v7, const char *f, val_t e) {
  char buf[512];
  char *s = v7_to_json(v7, e, buf, sizeof(buf));
  fprintf(stderr, "Exec error [%s]: %s\n", f, s);
  if (s != buf) {
    free(s);
  }
}

/*
 * V7 executable main function.
 * `init_func()` is an optional intialization function, aimed to export any
 * extra functionality into vanilla v7 engine.
 */
int v7_main(int argc, char *argv[], void (*init_func)(struct v7 *)) {
  struct v7 *v7 = v7_create();
  int i, show_ast = 0, binary_ast = 0;
  val_t res = v7_create_undefined();

  if (init_func != NULL) {
    init_func(v7);
  }

  /* Execute inline code */
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      if (show_ast) {
        v7_compile(argv[i + 1], binary_ast, stdout);
      } else if (v7_exec(v7, &res, argv[i + 1]) != V7_OK) {
        print_error(v7, argv[i + 1], res);
        res = v7_create_undefined();
      }
      i++;
    } else if (strcmp(argv[i], "-t") == 0) {
      show_ast = 1;
    } else if (strcmp(argv[i], "-b") == 0) {
      show_ast = 1;
      binary_ast = 1;
    } else if (strcmp(argv[i], "-h") == 0) {
      show_usage(argv);
    }
  }

  if (argc == 1) {
    show_usage(argv);
  }

  /* Execute files */
  for (; i < argc; i++) {
    if (show_ast) {
      size_t size;
      char *source_code;
      if ((source_code = read_file(argv[i], &size)) == NULL) {
        fprintf(stderr, "Cannot read [%s]\n", argv[i]);
      } else {
        v7_compile(source_code, binary_ast, stdout);
        free(source_code);
      }
    } else if (v7_exec_file(v7, &res, argv[i]) != V7_OK) {
      print_error(v7, argv[i], res);
      res = v7_create_undefined();
    }
  }

  if (!show_ast) {
    char buf[2000];
    v7_to_json(v7, res, buf, sizeof(buf));
    printf("%s\n", buf);
  }

  v7_destroy(v7);
  return EXIT_SUCCESS;
}
#endif

#ifdef V7_EXE
int main(int argc, char *argv[]) {
  return v7_main(argc, argv, NULL);
}
#endif
