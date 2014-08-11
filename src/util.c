static void inc_ref_count(struct v7_val *v) {
  assert(v != NULL);
  assert(v->ref_count >= 0);
  assert(!(v->flags & V7_VAL_DEALLOCATED));
  v->ref_count++;
}

static char *v7_strdup(const char *ptr, unsigned long len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) return NULL;
  memcpy(p, ptr, len);
  p[len] = '\0';
  return p;
}

static int instanceof(const struct v7_val *obj, const struct v7_val *ctor) {
  if (obj->type == V7_TYPE_OBJ && ctor != NULL) {
    while (obj->ctor != NULL) {
      if (obj->ctor == ctor) return 1;
      obj = obj->ctor;
      if (obj->ctor == obj) break;  // Break on circular reference
    }
  }
  return 0;
}

int v7_is_class(const struct v7_val *obj, enum v7_class cls) {
  return instanceof(obj, &s_constructors[cls]);
}

static struct v7_val *common_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : cfa->result;
  obj->type = V7_TYPE_OBJ;
  return obj;
}

void v7_init_str(struct v7_val *v, char *p, unsigned long len, int own) {
  v->type = V7_TYPE_STR;
  v->proto = &s_prototypes[V7_CLASS_STRING];
  v->v.str.buf = p;
  v->v.str.len = len;
  v->flags &= ~V7_STR_ALLOCATED;
  if (own) {
    if (len < sizeof(v->v.str.loc) - 1) {
      v->v.str.buf = v->v.str.loc;
      memcpy(v->v.str.loc, p, len);
      v->v.str.loc[len] = '\0';
    } else {
      v->v.str.buf = v7_strdup(p, len);
      v->flags |= V7_STR_ALLOCATED;
    }
  }
}

void v7_init_num(struct v7_val *v, double num) {
  v->type = V7_TYPE_NUM;
  v->proto = &s_prototypes[V7_CLASS_NUMBER];
  v->v.num = num;
}

void v7_init_bool(struct v7_val *v, int is_true) {
  v->type = V7_TYPE_BOOL;
  v->proto = &s_prototypes[V7_CLASS_BOOLEAN];
  v->v.num = is_true ? 1.0 : 0.0;
}

void v7_init_func(struct v7_val *v, v7_c_func_t func) {
  v7_set_class(v, V7_CLASS_FUNCTION);
  v->v.c_func = func;
}
