/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#ifdef V7_EXE

/*
 * $ ./v7 --show-ast -e 'var foo = 1;' jquery.js
 */
static const char *static_text_ast_flag = "--dump-text-ast";
static const char *static_bin_ast_flag = "--dump-binary-ast";

static void show_usage(char *argv[]) {
  fprintf(stderr, "Usage: %s [%s] [%s] [-e expression ...] js_file ...\n",
          argv[0], static_text_ast_flag, static_bin_ast_flag);
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
    fwrite(ast.buf, ast.len, 1, stdout);
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
      } else if (!v7_exec(v7, argv[i + 1])) {
        fprintf(stderr, "Error executing [%s]: %s\n", argv[i + 1],
                v7_get_error_string(v7));
      }
      i++;
    } else if (strcmp(argv[i], static_text_ast_flag) == 0) {
      show_ast = 1;
    } else if (strcmp(argv[i], static_bin_ast_flag) == 0) {
      show_ast = 1;
      binary_ast = 1;
    } else if (strcmp(argv[i], "-h") == 0) {
      show_usage(argv);
    }
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
    } else if (!v7_exec_file(v7, argv[i])) {
      fprintf(stderr, "%s\n", v7_get_error_string(v7));
    }
  }

  v7_destroy(&v7);
  return EXIT_SUCCESS;
}
#endif
