static void Error_ctor(struct v7_c_func_arg *cfa) {
  common_ctor(cfa);
}

static void init_error(void) {
  SET_RO_PROP_V(s_global, "Error", s_constructors[V7_CLASS_ERROR]);
}
