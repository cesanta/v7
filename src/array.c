static void Array_ctor(struct v7_c_func_arg *cfa) {
  common_ctor(cfa);
}
static void Arr_length(struct v7_val *this_obj, struct v7_val *result) {
  struct v7_prop *p;
  v7_init_num(result, 0.0);
  for (p = this_obj->v.array; p != NULL; p = p->next) {
    result->v.num += 1.0;
  }
}

static void Arr_push(struct v7_c_func_arg *cfa) {
  int i;
  for (i = 0; i < cfa->num_args; i++) {
    v7_append(cfa->v7, cfa->this_obj, cfa->args[i]);
  }
}

static int cmp_prop(const void *pa, const void *pb) {
  const struct v7_prop *p1 = * (struct v7_prop **) pa;
  const struct v7_prop *p2 = * (struct v7_prop **) pb;
  return cmp(p2->val, p1->val);
}

static void Arr_sort(struct v7_c_func_arg *cfa) {
  int i = 0, length = 0;
  struct v7_val *v = cfa->this_obj;
  struct v7_prop *p, **arr;

  // TODO(lsm): do proper error checking
  for (p = v->v.array; p != NULL; p = p->next) {
    length++;
  }
  arr = (struct v7_prop **) malloc(length * sizeof(p));
  for (i = 0, p = v->v.array; p != NULL; p = p->next) {
    arr[i++] = p;
  }
  qsort(arr, length, sizeof(p), cmp_prop);
  v->v.array = NULL;
  for (i = 0; i < length; i++) {
    arr[i]->next = v->v.array;
    v->v.array = arr[i];
  }
  free(arr);
}

static void arr_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m, *head = v->v.array;
  int n = snprintf(buf, bsiz, "%s", "[");

  for (m = head; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != head) n += snprintf(buf + n , bsiz - n, "%s", ", ");
    v7_to_string(m->val, buf + n, bsiz - n);
    n = (int) strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "]");
}

