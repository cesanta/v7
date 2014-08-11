static void Function_ctor(struct v7_c_func_arg *cfa) {
  common_ctor(cfa);
}

static void init_function(void) {
  SET_RO_PROP_V(s_global, "Function", s_constructors[V7_CLASS_FUNCTION]);
}
