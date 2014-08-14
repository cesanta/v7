static enum v7_err Json_stringify(struct v7_c_func_arg *cfa) {
  v7_init_str(cfa->result, NULL, 0, 0);
  // TODO(lsm): implement JSON.stringify
  return V7_OK;
}

static void init_json(void) {
  SET_METHOD(s_json, "stringify", Json_stringify);

  v7_set_class(&s_json, V7_CLASS_OBJECT);
  s_json.ref_count = 1;

  SET_RO_PROP_V(s_global, "JSON", s_json);
}