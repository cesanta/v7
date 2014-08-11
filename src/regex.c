static void Regex_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = common_ctor(cfa);
  v7_set_class(obj, V7_CLASS_REGEXP);
}

static void init_regex(void) {
  SET_RO_PROP_V(s_global, "RegExp", s_constructors[V7_CLASS_REGEXP]);
}
