/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

enum v7_type v7_value_type(struct v7 *v7, v7_value_t v) {
  if (v7_is_double(v)) {
    return V7_TYPE_NUMBER;
  }
  switch (v & V7_TAG_MASK) {
    case V7_TAG_FOREIGN:
      if (v == V7_NULL) {
        return V7_TYPE_NULL;
      }
      return V7_TYPE_FOREIGN;
    case V7_TAG_UNDEFINED:
      return V7_TYPE_UNDEFINED;
    case V7_TAG_OBJECT:
      if (v7_value_to_object(v)->prototype == v7->array_prototype) {
        return V7_TYPE_ARRAY_OBJECT;
      } else {
        return V7_TYPE_GENERIC_OBJECT;
      }
    case V7_TAG_STRING:
      return V7_TYPE_STRING;
    case V7_TAG_BOOLEAN:
      return V7_TYPE_BOOLEAN;
    default:
      /* TODO(mkm): or should we crash? */
      return V7_TYPE_UNDEFINED;
  }
}

int v7_is_double(v7_value_t v) {
  v7_value_t nan;
  V7_SET_NAN(nan);
  return !(isnan(v7_value_to_double(v)) && v != nan);
}

int v7_is_object(v7_value_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_OBJECT;
}

int v7_is_string(v7_value_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_STRING;
}

int v7_is_boolean(v7_value_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_BOOLEAN;
}

int v7_is_null(v7_value_t v) {
  return v == V7_NULL;
}

int v7_is_undefined(v7_value_t v) {
  return v == V7_UNDEFINED;
}

V7_PRIVATE v7_value_t v7_pointer_to_value(void *p) {
  return ((uint64_t) p & ((1L << 48) -1));
}

V7_PRIVATE void *v7_value_to_pointer(v7_value_t v) {
  struct {
    uint64_t s:48;
  } h;
  return (void *) (uintptr_t) (h.s = v);
}

v7_value_t v7_object_to_value(struct v7_object *o) {
  return v7_pointer_to_value(o) | V7_TAG_OBJECT;
}

struct v7_object *v7_value_to_object(v7_value_t v) {
  return (struct v7_object *) v7_value_to_pointer(v);
}

v7_value_t v7_string_to_value(struct v7_str *s) {
  return v7_pointer_to_value(s) | V7_TAG_STRING;
}

struct v7_str *v7_value_to_string(v7_value_t v) {
  return (struct v7_str *) v7_value_to_pointer(v);
}

v7_value_t v7_foreign_to_value(void *p) {
  return v7_pointer_to_value(p) | V7_TAG_FOREIGN;
}

void *v7_value_to_foreign(v7_value_t v) {
  return v7_value_to_pointer(v);
}

v7_value_t v7_boolean_to_value(int v) {
  return (!!v) | V7_TAG_BOOLEAN;
}

int v7_value_to_boolean(v7_value_t v) {
  return v & 1;
}

v7_value_t v7_double_to_value(double v) {
  v7_value_t res;
  /* not every NaN is a JS NaN */
  if (isnan(v)) {
    V7_SET_NAN(res);
  } else {
    * (double *) &res = v;
  }
  return res;
}

double v7_value_to_double(v7_value_t v) {
  return * (double *) &v;
}

static v7_value_t v7_create_object(struct v7 *v7, struct v7_object *proto) {
  /* TODO(mkm): use GC heap */
  struct v7_object *o = (struct v7_object *) malloc(sizeof(struct v7_object));
  if (o == NULL) {
    return V7_NULL;
  }
  (void) v7;
  o->properties = NULL;
  o->prototype = proto;
  return v7_object_to_value(o);
}

v7_value_t v7_create_value(struct v7 *v7, enum v7_type type, ...) {
  va_list ap;
  va_start(ap, type);
  return v7_va_create_value(v7, type, ap);
}

v7_value_t v7_va_create_value(struct v7 *v7, enum v7_type type,
                            va_list ap) {
  v7_value_t v;
  const char *str;

  /* Initialize value based on type */
  switch (type) {
    case V7_TYPE_NULL:
      return V7_NULL;
    case V7_TYPE_UNDEFINED:
      return V7_UNDEFINED;
    case V7_TYPE_NUMBER:
      return v7_double_to_value(va_arg(ap, double));
    case V7_TYPE_BOOLEAN:
      return v7_boolean_to_value(va_arg(ap, int));
    case V7_TYPE_STRING:
      /* TODO(mkm) this will be way different */
      {
        struct v7_str *s = (struct v7_str *) malloc(sizeof(struct v7_str));
        if (s == NULL) {
          return V7_NULL;
        }
        str = va_arg(ap, char *);
        s->len = va_arg(ap, size_t);
        s->buf = malloc(s->len);
        strncpy(s->buf, str, s->len);
        return v7_string_to_value(s);
      }
    case V7_TYPE_GENERIC_OBJECT:
      return v7_create_object(v7, v7->object_prototype);
    case V7_TYPE_ARRAY_OBJECT:
      return v7_create_object(v7, v7->array_prototype);
    case V7_TYPE_BOOLEAN_OBJECT:
    case V7_TYPE_STRING_OBJECT:
    case V7_TYPE_NUMBER_OBJECT:
    case V7_TYPE_FUNCION_OBJECT:
    case V7_TYPE_C_FUNCION_OBJECT:
    case V7_TYPE_REGEXP_OBJECT:
    case V7_TYPE_DATE_OBJECT:
    case V7_TYPE_ERROR_OBJECT:
    default:
      printf("NOT IMPLEMENTED YET\n");
      abort();
  }

  return v;
}

int v7_to_json(struct v7 *v7, v7_value_t v, char *buf, size_t size) {
  /* TODO(mkm): call the toString method instead of custom C code. */
  switch (v7_value_type(v7, v)) {
    case V7_TYPE_NULL:
      return stpncpy(buf, "null", size) - buf;
    case V7_TYPE_UNDEFINED:
      return stpncpy(buf, "undefined", size) - buf;
    case V7_TYPE_BOOLEAN:
      return stpncpy(buf,
                     v7_value_to_boolean(v) ? "true" : "false", size) - buf;
    case V7_TYPE_NUMBER:
      return snprintf(buf, size, "%lg", v7_value_to_double(v));
    case V7_TYPE_STRING:
      {
        struct v7_str *str = v7_value_to_string(v);
        return snprintf(buf, size, "\"%.*s\"", (int) str->len, str->buf);
      }
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
        for (p = v7_value_to_object(v)->properties;
             p && (size - (b - buf)); p = p->next) {
          b += snprintf(b, size - (b - buf), "\"%s\":", p->name);
          b += v7_to_json(v7, p->value, b, size - (b - buf));
          if (p->next) {
            b += snprintf(b, size - (b - buf), ",");
          }
        }
        b += snprintf(b, size - (b - buf), "}");
        return b - buf;
      }
    case V7_TYPE_ARRAY_OBJECT:
      {
        struct v7_property *p;
        char *b = buf;
        char key[512];
        size_t i, len;
        v7_value_t lenv = v7_array_length(v7, v);
        if (lenv == V7_UNDEFINED) {
          return 0;
        }
        len = (size_t) v7_value_to_double(lenv);
        b += snprintf(b, size - (b - buf), "[");
        for (i = 0; i < len; i++) {
          /* TODO */
          snprintf(key, sizeof(key), "%lu", i);
          if ((p = v7_get_property(v, key, -1)) != NULL) {
            b += v7_to_json(v7, p->value, b, size - (b - buf));
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

int v7_stringify_value(struct v7 *v7, v7_value_t v, char *buf,
                       size_t size) {
  if (v7_is_string(v)) {
    char *b;
    struct v7_str *str = v7_value_to_string(v);
    v7_strlen_t len = str->len;
    if (len > size - 1) {
      len = size - 1;
    }
    b = stpncpy(buf, str->buf, len);
    buf[str->len] = '\0';
    return b - buf;
  } else {
    return v7_to_json(v7, v, buf, size);
  }
}

static struct v7_property *v7_create_property(struct v7 *v7) {
  /* TODO(mkm): allocate from GC pool */
  (void) v7;
  return (struct v7_property *) calloc(1, sizeof(struct v7_property));
}

struct v7_property *v7_get_property(v7_value_t obj,
                                    const char *name,
                                    v7_strlen_t len) {
  struct v7_property *prop;

  if (!v7_is_object(obj)) {
    return NULL;
  }
  if (len == (v7_strlen_t) -1) {
    len = strlen(name);
  }
  for (prop = v7_value_to_object(obj)->properties; prop != NULL;
       prop = prop->next) {
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

int v7_set_property_value(struct v7 *v7, v7_value_t obj,
                          const char *name, v7_strlen_t len,
                          unsigned int attributes,
                          v7_value_t val) {
  struct v7_property *prop;

  if (!v7_is_object(obj)) {
    return -1;
  }

  prop = v7_get_property(obj, name, len);
  if (prop == NULL) {
    if ((prop = v7_create_property(v7)) == NULL) {
      return -1;
    }
    prop->next = v7_value_to_object(obj)->properties;
    v7_value_to_object(obj)->properties = prop;
  }

  prop->attributes = attributes;
  if (len == (v7_strlen_t) ~0) {
    len = strlen(name);
  }
  prop->name = malloc(len + 1);
  strncpy(prop->name, name, len);
  prop->name[len] = '\0';

  prop->value = val;
  return 0;
}

int v7_set_property(struct v7 *v7, v7_value_t obj, const char *name,
                    v7_strlen_t len, unsigned int attributes,
                    enum v7_type type, ...) {
  v7_value_t val;
  va_list ap;
  int res;

  va_start(ap, type);
  val = v7_va_create_value(v7, type, ap);
  res = v7_set_property_value(v7, obj, name, len, attributes, val);
  return res;
}

int v7_del_property(v7_value_t obj, const char *name, v7_strlen_t len) {
  struct v7_property *prop, *prev;

  if (!v7_is_object(obj)) {
    return -1;
  }
  if (len == (v7_strlen_t) ~0) {
    len = strlen(name);
  }
  for (prev = NULL, prop = v7_value_to_object(obj)->properties; prop != NULL;
       prev = prop, prop = prop->next) {
    if (strncmp(prop->name, name, len) == 0) {
      if (prev) {
        prev->next = prop->next;
      } else {
        v7_value_to_object(obj)->properties = prop->next;
      }
      v7_destroy_property(&prop);
      return 0;
    }
  }
  return -1;
}

V7_PRIVATE v7_value_t v7_property_value(struct v7_property *p) {
  if (p == NULL) {
    return V7_UNDEFINED;
  }
  return p->value;
}

V7_PRIVATE v7_value_t v7_array_length(struct v7 *v7, v7_value_t v) {
  struct v7_property *prop;
  long max = -1, k;
  char *end;

  /* TODO(mkm) new values don't have yet an array object type */
  if (v7_value_type(v7, v) != V7_TYPE_ARRAY_OBJECT) {
    return v7_create_value(v7, V7_TYPE_UNDEFINED);
  }
  for (prop = v7_value_to_object(v)->properties; prop != NULL;
       prop = prop->next) {
    k = strtol(prop->name, &end, 10);
    if (end != prop->name && k > max) {
      max = k;
    }
  }
  return v7_create_value(v7, V7_TYPE_NUMBER, (double) (max + 1));
}
