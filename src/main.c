/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#ifdef V7_EXE

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
      fread(data, 1, *size, fp);
      data[*size] = '\0';
    }
    fclose(fp);
  }
  return data;
}

static void dump_ast(const char *code, int binary) {
  struct ast ast;

  ast_init(&ast, 0);
  if (aparse(&ast, code, 1) != V7_OK) {
    fprintf(stderr, "%s\n", "parse error");
  } else if (binary) {
    fwrite(ast.mbuf.buf, ast.mbuf.len, 1, stdout);
  } else {
    ast_dump(stdout, &ast, 0);
  }
  ast_free(&ast);
}

int main(int argc, char *argv[]) {
  struct v7 *v7 = v7_create();
  int i, show_ast = 0, binary_ast = 0;

  /* Execute inline code */
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      if (show_ast) {
        dump_ast(argv[i + 1], binary_ast);
      } else if (v7_exec(v7, argv[i + 1]) == V7_UNDEFINED) {
        fprintf(stderr, "Exec error [%s]: %s\n", argv[i + 1], v7->error_msg);
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
        dump_ast(source_code, binary_ast);
        free(source_code);
      }
    } else {
      size_t len;
      char *code = read_file(argv[i], &len);
      if (code == NULL) {
        fprintf(stderr, "Cannot open %s: %s\n", argv[i], strerror(errno));
      } else if (v7_exec(v7, code) == V7_UNDEFINED) {
          fprintf(stderr, "Exec error [%s]: %s\n", argv[i], v7->error_msg);
      }
    }
  }

  v7_destroy(v7);
  return EXIT_SUCCESS;
}
#endif
