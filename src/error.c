static enum v7_err Error_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : cfa->result;
  v7_set_class(obj, V7_CLASS_ERROR);
  return V7_OK;
}

static void init_error(void) {
  SET_RO_PROP_V(s_global, "Error", s_constructors[V7_CLASS_ERROR]);
}
