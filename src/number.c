static void Number_ctor(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, cfa->num_args > 0 ? cfa->args[0]->v.num : 0.0);
  if (cfa->called_as_constructor) {
    cfa->this_obj->proto = &s_prototypes[V7_CLASS_NUMBER];
    cfa->this_obj->ctor = &s_constructors[V7_CLASS_NUMBER];
    cfa->this_obj->v.num = cfa->result->v.num;
  }
}

static void Num_toFixed(struct v7_c_func_arg *cfa) {
  int len, digits = cfa->num_args > 0 ? (int) cfa->args[0]->v.num : 0;
  char fmt[10], buf[100];

  snprintf(fmt, sizeof(fmt), "%%.%dlf", digits);
  len = snprintf(buf, sizeof(buf), fmt, cfa->this_obj->v.num);
  v7_init_str(cfa->result, buf, len, 1);
}

static void init_number(void) {
  SET_RO_PROP(s_constructors[V7_CLASS_NUMBER], "MAX_VALUE",
              V7_TYPE_NUM, num, LONG_MAX);
  SET_RO_PROP(s_constructors[V7_CLASS_NUMBER], "MIN_VALUE",
              V7_TYPE_NUM, num, LONG_MIN);
  SET_RO_PROP(s_constructors[V7_CLASS_NUMBER], "NaN",
              V7_TYPE_NUM, num, NAN);
  SET_METHOD(s_prototypes[V7_CLASS_NUMBER], "toFixed", Num_toFixed);
  SET_RO_PROP_V(s_global, "Number", s_constructors[V7_CLASS_NUMBER]);
}
