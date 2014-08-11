static void Object_ctor(struct v7_c_func_arg *cfa) {
  common_ctor(cfa);
}

static void obj_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m, *head = v->props;
  int n = snprintf(buf, bsiz, "%s", "{");

  for (m = head; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != head) n += snprintf(buf + n , bsiz - n, "%s", ", ");
    v7_to_string(m->key, buf + n, bsiz - n);
    n = (int) strlen(buf);
    n += snprintf(buf + n , bsiz - n, "%s", ": ");
    v7_to_string(m->val, buf + n, bsiz - n);
    n = (int) strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "}");
}

static void Obj_toString(struct v7_c_func_arg *cfa) {
  char buf[4000];
  v7_to_string(cfa->this_obj, buf, sizeof(buf));
  v7_init_str(cfa->result, buf, strlen(buf), 1);
}

static void Obj_keys(struct v7_c_func_arg *cfa) {
  struct v7_prop *p;
  v7_set_class(cfa->result, V7_CLASS_ARRAY);
  for (p = cfa->this_obj->props; p != NULL; p = p->next) {
    v7_append(cfa->v7, cfa->result, p->key);
  }
}

static void init_object(void) {
  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "toString", Obj_toString);
  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "keys", Obj_keys);
  SET_RO_PROP_V(s_global, "Object", s_constructors[V7_CLASS_OBJECT]);
}
