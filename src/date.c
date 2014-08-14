static enum v7_err Date_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : cfa->result;
  v7_set_class(obj, V7_CLASS_DATE);
  return V7_OK;
}

static void init_date(void) {
  SET_RO_PROP_V(s_global, "Date", s_constructors[V7_CLASS_DATE]);
}
