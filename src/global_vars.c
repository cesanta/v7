static const int s_op_lengths[NUM_OPS] = {
  -1,
  1, 1, 2, 2,
  2, 2, 3, 3,
  1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4
};

static struct v7_val s_constructors[V7_NUM_CLASSES];
static struct v7_val s_prototypes[V7_NUM_CLASSES];
static struct v7_val s_global = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
static struct v7_val s_json = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
static struct v7_val s_math = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
