#include "internal.h"


V7_PRIVATE enum v7_err Regex_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  struct v7_val *str = cfa->args[0];
  struct v7 *v7 = cfa->v7;  // Needed for TRY() macro below

  if(cfa->num_args > 0){
    if(!is_string(str)){
      TRY(toString(cfa->v7, str));
      str = v7_top_val(cfa->v7);
    }
    // TODO: copy str --> regex

    if(cfa->num_args > 1){
      struct v7_val *flags = cfa->args[1];
      if(!is_string(flags)){
        TRY(toString(cfa->v7, flags));
        flags = v7_top_val(cfa->v7);
      }
      uint32_t ind = flags->v.str.len;
      while(ind){
        switch(flags->v.str.buf[--ind]){
          case 'g': obj->regexp_fl_g=1;  break;
          case 'i': obj->regexp_fl_i=1;  break;
          case 'm': obj->regexp_fl_m=1;  break;
        }
      }
    }
  }

  v7_set_class(obj, V7_CLASS_REGEXP);
  return V7_OK;
}

V7_PRIVATE void Regex_global(struct v7_val *this_obj, struct v7_val *result) {
  v7_init_bool(result, this_obj->regexp_fl_g);
}

V7_PRIVATE void Regex_ignoreCase(struct v7_val *this_obj, struct v7_val *result) {
  v7_init_bool(result, this_obj->regexp_fl_i);
}

V7_PRIVATE void Regex_multiline(struct v7_val *this_obj, struct v7_val *result) {
  v7_init_bool(result, this_obj->regexp_fl_m);
}

V7_PRIVATE void init_regex(void) {
  init_standard_constructor(V7_CLASS_REGEXP, Regex_ctor);
  SET_RO_PROP_V(s_global, "RegExp", s_constructors[V7_CLASS_REGEXP]);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "global", Regex_global);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "ignoreCase", Regex_ignoreCase);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "multiline", Regex_multiline);
}
