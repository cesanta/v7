static enum v7_err Boolean_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : cfa->result;
  v7_set_class(obj, V7_CLASS_BOOLEAN);
  return V7_OK;
}