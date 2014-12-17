/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

struct v7_value *v7_create_value(struct v7 *v7, enum v7_type type, ...) {
  struct v7_value *v;

  /* TODO(lsm): take object from a free object cache */
  (void) v7;

  if ((v = (struct v7_value *) calloc(1, sizeof(*v))) != NULL) {
    /* Initialize value based on type */
    (void) type;
  }

  return v;
}

static struct v7_property *v7_create_property(struct v7 *v7) {
  /* TODO(lsm): take property from a cache */
  (void) v7;
  return (struct v7_property *) calloc(1, sizeof(struct v7_property));
}

struct v7_property *v7_get_property(struct v7_value *obj, const char *name,
                                    v7_strlen_t len) {
  struct v7_property *prop;

  if (obj == NULL || obj->type < V7_TYPE_GENERIC_OBJECT) {
    return NULL;
  }

  for (prop = obj->value.object->properties; prop != NULL; prop = prop->next) {
    if (strncmp(prop->name, name, len) == 0) {
      return prop;
    }
  }

  return NULL;
}

int v7_set_property(struct v7 *v7, struct v7_value *obj, const char *name,
                    v7_strlen_t len, unsigned int attributes,
                    enum v7_type type, ...) {
  struct v7_property *prop;

  if ((prop = v7_create_property(v7)) != NULL) {
    prop->attributes = attributes;
    prop->name = malloc(len + 1);
    strncpy(prop->name, name, len);

    /* Initialize value based on type */
    (void) type;

    /* TODO(lsm): make sure the property is unique */
    prop->next = obj->value.object->properties;
    obj->value.object->properties = prop;
  }

  return 0;
}
