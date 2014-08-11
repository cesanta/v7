static void Date_ctor(struct v7_c_func_arg *cfa) {
  common_ctor(cfa);
}

static void init_date(void) {
  SET_RO_PROP_V(s_global, "Date", s_constructors[V7_CLASS_DATE]);
}
