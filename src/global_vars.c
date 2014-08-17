#include "internal.h"

V7_PRIVATE struct v7_val s_constructors[V7_NUM_CLASSES];
V7_PRIVATE struct v7_val s_prototypes[V7_NUM_CLASSES];

V7_PRIVATE struct v7_val s_global = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
V7_PRIVATE struct v7_val s_math = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
V7_PRIVATE struct v7_val s_json = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
V7_PRIVATE struct v7_val s_file = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
