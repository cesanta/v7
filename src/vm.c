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
  const char *str;

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
      str = va_arg(ap, char *);
      v->value.string.len = va_arg(ap, size_t);
      v->value.string.buf = malloc(v->value.string.len);
      strncpy(v->value.string.buf, str, v->value.string.len);
      break;
    case V7_TYPE_GENERIC_OBJECT:
    case V7_TYPE_BOOLEAN_OBJECT:
    case V7_TYPE_STRING_OBJECT:
    case V7_TYPE_NUMBER_OBJECT:
    case V7_TYPE_FUNCION_OBJECT:
    case V7_TYPE_C_FUNCION_OBJECT:
    case V7_TYPE_REGEXP_OBJECT:
    case V7_TYPE_ARRAY_OBJECT:
    case V7_TYPE_DATE_OBJECT:
    case V7_TYPE_ERROR_OBJECT:
      v->value.object = (struct v7_object *) calloc(
          1, sizeof(struct v7_object));
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

int v7_to_json(struct v7 *v7, struct v7_value *v, char *buf, size_t size) {
  /* TODO(mkm): call the toString method instead of custom C code. */
  switch (v->type) {
    case V7_TYPE_NULL:
      return stpncpy(buf, "null", size) - buf;
    case V7_TYPE_UNDEFINED:
      return stpncpy(buf, "undefined", size) - buf;
    case V7_TYPE_BOOLEAN:
      return stpncpy(buf, v->value.boolean?"true":"false", size) - buf;
    case V7_TYPE_NUMBER:
      return snprintf(buf, size, "%lg", v->value.number);
    case V7_TYPE_STRING:
      return snprintf(buf, size, "\"%.*s\"", (int) v->value.string.len,
                      v->value.string.buf);
    case V7_TYPE_GENERIC_OBJECT:
    case V7_TYPE_BOOLEAN_OBJECT:
    case V7_TYPE_STRING_OBJECT:
    case V7_TYPE_NUMBER_OBJECT:
    case V7_TYPE_FUNCION_OBJECT:
    case V7_TYPE_C_FUNCION_OBJECT:
    case V7_TYPE_REGEXP_OBJECT:
    case V7_TYPE_DATE_OBJECT:
    case V7_TYPE_ERROR_OBJECT:
      {
        char *b = buf;
        struct v7_property *p;
        b += snprintf(b, size - (b - buf), "{");
        for (p = v->value.object->properties;
             p && (size - (b - buf)); p = p->next) {
          b += snprintf(b, size - (b - buf), "\"%s\":", p->name);
          b += v7_to_json(v7, &p->value, b, size - (b - buf));
          if (p->next) {
            b += snprintf(b, size - (b - buf), ",");
          }
        }
        b += snprintf(b, size - (b - buf), "}");
        return b - buf;
      }
    case V7_TYPE_ARRAY_OBJECT:
      {
        char *b = buf;
        size_t i, len = (size_t) v7_array_length(v7, v)->value.number;
        char key[512];
        struct v7_property *p;
        b += snprintf(b, size - (b - buf), "[");
        for (i = 0; i < len; i++) {
          /* TODO */
          snprintf(key, sizeof(key), "%lu", i);
          if ((p = v7_get_property(v, key, -1)) != NULL) {
            b += v7_to_json(v7, &p->value, b, size - (b - buf));
          }
          if (i != len - 1) {
            b += snprintf(b, size - (b - buf), ",");
          }
        }
        b += snprintf(b, size - (b - buf), "]");
        return b - buf;
      }
    default:
      printf("NOT IMPLEMENTED YET\n");
      abort();
  }
}

int v7_stringify_value(struct v7 *v7, struct v7_value *v, char *buf,
                       size_t size) {
  if (v->type == V7_TYPE_STRING) {
    v7_strlen_t len = v->value.string.len;
    if (len > size - 1) {
      len = size - 1;
    }
    char *b = stpncpy(buf, v->value.string.buf, len);
    buf[v->value.string.len] = '\0';
    return b - buf;
  } else {
    return v7_to_json(v7, v, buf, size);
  }
}

static struct v7_property *v7_create_property(struct v7 *v7) {
  /* TODO(lsm): take property from a cache */
  (void) v7;
  return (struct v7_property *) calloc(1, sizeof(struct v7_property));
}

struct v7_property *v7_get_property(struct v7_value *obj,
                                    const char *name,
                                    v7_strlen_t len) {
  struct v7_property *prop;

  if (obj == NULL || !v7_is_object(obj)) {
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

static void v7_destroy_property(struct v7_property **p) {
  free((*p)->name);
  free(*p);
  *p = NULL;
}

int v7_set_property_value(struct v7 *v7, struct v7_value *obj,
                          const char *name, v7_strlen_t len,
                          unsigned int attributes,
                          struct v7_value *val) {
  struct v7_property *prop;

  if (obj == NULL || !v7_is_object(obj)) {
    return -1;
  }

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
  prop->name[len] = '\0';

  prop->value = *val;
  return 0;
}

int v7_set_property(struct v7 *v7, struct v7_value *obj, const char *name,
                    v7_strlen_t len, unsigned int attributes,
                    enum v7_type type, ...) {
  struct v7_value *val;
  va_list ap;
  int res;

  va_start(ap, type);
  /* TODO(mkm): avoid this extra allocation */
  val = v7_va_create_value(v7, type, ap);
  res = v7_set_property_value(v7, obj, name, len, attributes, val);
  free(val);
  return res;
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
      v7_destroy_property(&prop);
      return 0;
    }
  }
  return -1;
}

V7_PRIVATE struct v7_value *v7_property_value(struct v7_property *p) {
  if (p == NULL) {
    return NULL;
  }
  return &p->value;
}

V7_PRIVATE int v7_is_object(struct v7_value *v) {
  return v->type >= V7_TYPE_GENERIC_OBJECT && v->type < V7_TYPE_MAX_OBJECT_TYPE;
}

V7_PRIVATE struct v7_value *v7_array_length(struct v7 *v7, struct v7_value *v) {
  struct v7_property *prop;
  long max = -1, k;
  char *end;

  if (v->type != V7_TYPE_ARRAY_OBJECT) {
    return v7_create_value(v7, V7_TYPE_UNDEFINED);
  }
  for (prop = v->value.object->properties; prop != NULL; prop = prop->next) {
    k = strtol(prop->name, &end, 10);
    if (end != prop->name && k > max) {
      max = k;
    }
  }
  return v7_create_value(v7, V7_TYPE_NUMBER, (double) (max + 1));
}
