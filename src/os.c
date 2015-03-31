/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#ifndef V7_NO_FS
/*
 * File interface: a wrappers around open(), close(), read(), write().
 * OS.open(path, flags[, mode]) -> fd.
 * OS.close(fd) -> undefined
 * OS.read(fd) -> string (empty string on EOF)
 * OS.write(fd, str) -> num_bytes_written
 * OS.remove(str) -> errno
 */
static val_t OS_read(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  char buf[BUFSIZ];

  (void) this_obj;
  if (v7_is_double(arg0)) {
    int fd = v7_to_double(arg0);
    int n = read(fd, buf, sizeof(buf));
    if (n > 0) {
      return v7_create_string(v7, buf, n, 1);
    }
  }

  return v7_create_string(v7, "", 0, 1);
}

static val_t OS_write(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t arg1 = v7_array_get(v7, args, 1);
  size_t sent = 0, len = 0;

  (void) this_obj;
  if (v7_is_double(arg0) && v7_is_string(arg1)) {
    const char *s = v7_to_string(v7, &arg1, &len);
    int fd = v7_to_double(arg0), n;
    while (sent < len && (n = write(fd, s + sent, len - sent)) > 0) {
      sent += n;
    }
  }

  return v7_create_number(sent == len ? 0 : errno);
}

static val_t OS_close(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  int res = -1;
  (void) this_obj;
  if (v7_is_double(arg0)) {
    res = close((int) v7_to_double(arg0));
  }
  return v7_create_number(res);
}

static val_t OS_remove(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  int res = -1;
  (void) this_obj;
  if (v7_is_string(arg0)) {
    size_t n;
    const char *path = v7_to_string(v7, &arg0, &n);
    res = remove(path);
  }
  return v7_create_number(res == 0 ? 0 : errno);
}

static val_t OS_open(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t arg1 = v7_array_get(v7, args, 1);
  val_t arg2 = v7_array_get(v7, args, 2);
  int fd = -1;

  (void) this_obj;
  if (v7_is_string(arg0)) {
    size_t n1;
    const char *s = v7_to_string(v7, &arg0, &n1);
    int flags = v7_is_double(arg1) ? (int) v7_to_double(arg1) : 0;
    int mode = v7_is_double(arg2) ? (int) v7_to_double(arg2) : 0;
    fd = open(s, flags, mode);
  }

  return v7_create_number(fd);
}
#endif

V7_PRIVATE void init_os(struct v7 *v7) {
  val_t os_obj = v7_create_object(v7);
  v7_set_property(v7, v7->global_object, "OS", 2, 0, os_obj);
#ifndef V7_NO_FS
  set_cfunc_obj_prop(v7, os_obj, "open", OS_open);
  set_cfunc_obj_prop(v7, os_obj, "close", OS_close);
  set_cfunc_obj_prop(v7, os_obj, "read", OS_read);
  set_cfunc_obj_prop(v7, os_obj, "write", OS_write);
  set_cfunc_obj_prop(v7, os_obj, "remove", OS_remove);
#endif
}
