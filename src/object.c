#include "internal.h"

V7_PRIVATE enum v7_err Object_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : cfa->result;
  v7_set_class(obj, V7_CLASS_OBJECT);
  return V7_OK;
}

V7_PRIVATE enum v7_err Obj_toString(struct v7_c_func_arg *cfa) {
  char *p, buf[500];
  p = v7_stringify(cfa->this_obj, buf, sizeof(buf));
  v7_init_str(cfa->result, buf, strlen(buf), 1);
  if (p != buf) free(p);
  return V7_OK;
}

V7_PRIVATE enum v7_err Obj_keys(struct v7_c_func_arg *cfa) {
  struct v7_prop *p;
  v7_set_class(cfa->result, V7_CLASS_ARRAY);
  for (p = cfa->this_obj->props; p != NULL; p = p->next) {
    v7_append(cfa->v7, cfa->result, p->key);
  }
  return V7_OK;
}

V7_PRIVATE void init_object(void) {
  init_standard_constructor(V7_CLASS_OBJECT, Object_ctor);
  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "toString", Obj_toString);
  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "keys", Obj_keys);
  SET_RO_PROP_V(s_global, "Object", s_constructors[V7_CLASS_OBJECT]);
}
