static void Object_ctor(struct v7_c_func_arg *cfa) {
  common_ctor(cfa);
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
