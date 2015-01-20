/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>
#include "v7.h"

int main(void) {
  size_t n;
  const char *domain_str;
  struct v7 *v7 = v7_create();
  v7_val_t domain, port0, config;

  /* Load JSON configuration */
  if (v7_exec_file(v7, &config, "config.json") != V7_OK) {
    printf("%s\n", "Cannot load JSON config");
    return 1;
  }

  /* Lookup values in JSON configuration object */
  domain = v7_get(config, "domain", 6);
  port0 = v7_array_at(v7, v7_get(config, "ports", 5), 0);
  domain_str = v7_to_string(v7, &domain, &n);

  printf("Domain: [%.*s], port 0: [%d]\n",
         (int) n, domain_str, (int) v7_to_double(port0));

  v7_destroy(v7);
  return 0;
}
