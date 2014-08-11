#ifdef V7_EXE
int main(int argc, char *argv[]) {
  struct v7 *v7 = v7_create();
  int i, error_code;

  // Execute inline code
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      if ((error_code = v7_exec(v7, argv[i + 1])) != V7_OK) {
        fprintf(stderr, "Error executing [%s]: %s\n", argv[i + 1],
                v7_strerror(error_code));
      }
      i++;
    }
  }

  // Execute files
  for (; i < argc; i++) {
    if ((error_code = v7_exec_file(v7, argv[i])) != V7_OK) {
      fprintf(stderr, "%s line %d: %s\n", argv[i], v7->line_no,
              v7_strerror(error_code));
    }
  }

  v7_destroy(&v7);
  return EXIT_SUCCESS;
}
#endif
