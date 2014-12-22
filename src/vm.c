/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

struct v7_value *v7_create_value(struct v7 *v7, enum v7_type type, ...) {
  va_list ap;
  va_start(ap, type);
  return v7_va_create_value(v7, type, ap);
}

struct v7_value *v7_va_create_value(struct v7 *v7, enum v7_type type,
                                    va_list ap) {
  struct v7_value *v;

  /* TODO(lsm): take object from a free object cache */
  (void) v7;

  if ((v = (struct v7_value *) calloc(1, sizeof(*v))) == NULL) {
    return NULL;
  }

  /* Initialize value based on type */
  v->type = type;
  switch (type) {
    case V7_TYPE_NULL:
    case V7_TYPE_UNDEFINED:
      break;
    case V7_TYPE_NUMBER:
      v->value.number = va_arg(ap, double);
      break;
    case V7_TYPE_BOOLEAN:
      v->value.boolean = va_arg(ap, int);
      break;
    case V7_TYPE_STRING:
      v->value.string.buf = va_arg(ap, char *);
      v->value.string.len = va_arg(ap, size_t);
      break;
    case V7_TYPE_GENERIC_OBJECT:
      v->value.object = (struct v7_object *) calloc(1,
                                                    sizeof(struct v7_object));
      if ( v->value.object == NULL) {
        return NULL;
      }
      break;
    default:
      printf("NOT IMPLEMENTED YET\n");
      abort();
  }

  return v;
}

void v7_stringify_value(struct v7_value *v, char *buf, size_t size) {
  switch (v->type) {
    case V7_TYPE_NULL:
      strncpy(buf, "null", size);
      break;
    case V7_TYPE_UNDEFINED:
      strncpy(buf, "undefined", size);
      break;
    case V7_TYPE_BOOLEAN:
      strncpy(buf, v->value.boolean?"true":"false", size);
      break;
    case V7_TYPE_NUMBER:
      snprintf(buf, size, "%lg", v->value.number);
      break;
    case V7_TYPE_STRING:
      snprintf(buf, size, "\"%.*s\"", (int) v->value.string.len,
               v->value.string.buf);
      break;
    default:
      printf("NOT IMPLEMENTED YET\n");
      abort();
  }
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
  if (len == (v7_strlen_t) -1) {
    len = strlen(name);
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
  struct v7_value *val;
  va_list ap;

  prop = v7_get_property(obj, name, len);
  if (prop == NULL) {
    if ((prop = v7_create_property(v7)) == NULL) {
      return -1;
    }
    prop->next = obj->value.object->properties;
    obj->value.object->properties = prop;
  }

  prop->attributes = attributes;
  if (len == (v7_strlen_t) ~0) {
    len = strlen(name);
  }
  prop->name = malloc(len + 1);
  strncpy(prop->name, name, len);

  va_start(ap, type);
  /* TODO(mkm): avoid this extra allocation */
  val = v7_va_create_value(v7, type, ap);
  prop->value = *val;
  free(val);

  return 0;
}

int v7_del_property(struct v7_value *obj, const char *name, v7_strlen_t len) {
  struct v7_property *prop, *prev;

  if (obj == NULL || obj->type < V7_TYPE_GENERIC_OBJECT) {
    return -1;
  }
  if (len == (v7_strlen_t) ~0) {
    len = strlen(name);
  }
  for (prev = NULL, prop = obj->value.object->properties; prop != NULL;
       prev = prop, prop = prop->next) {
    if (strncmp(prop->name, name, len) == 0) {
      if (prev) {
        prev->next = prop->next;
      } else {
        obj->value.object->properties = prop->next;
      }
      free(prop);
      return 0;
    }
  }
  return -1;
}
