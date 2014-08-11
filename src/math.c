static void Math_random(struct v7_c_func_arg *cfa) {
  srand((unsigned long) cfa->result);   // TODO: make better randomness
  v7_init_num(cfa->result, (double) rand() / RAND_MAX);
}

static void Math_sin(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, cfa->num_args == 1 ? sin(cfa->args[0]->v.num) : 0.0);
}

static void Math_sqrt(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, cfa->num_args == 1 ? sqrt(cfa->args[0]->v.num) : 0.0);
}

static void Math_tan(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, cfa->num_args == 1 ? tan(cfa->args[0]->v.num) : 0.0);
}

static void Math_pow(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, cfa->num_args == 2 ?
              pow(cfa->args[0]->v.num, cfa->args[1]->v.num) : 0.0);
}
