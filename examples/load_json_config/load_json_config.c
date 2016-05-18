/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>
#include "v7.h"

int v7_example(void) {
  size_t n;
  const char *domain_str;
  struct v7 *v7 = v7_create();
  v7_val_t domain, port0, config;

  /* Load JSON configuration */
  if (v7_parse_json_file(v7, "config.json", &config) != V7_OK) {
    printf("%s\n", "Cannot load JSON config");
    return 1;
  }

  /* Lookup values in JSON configuration object */
  domain = v7_get(v7, config, "domain", 6);
  port0 = v7_array_get(v7, v7_get(v7, config, "ports", 5), 0);
  domain_str = v7_get_string(v7, &domain, &n);

  printf("Domain: [%.*s], port 0: [%d]\n",
         (int) n, domain_str, (int) v7_get_double(v7, port0));

  v7_destroy(v7);
  return 0;
}
