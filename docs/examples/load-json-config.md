---
title: Load JSON config
---

```c
enum v7_err rcode = V7_OK;
v7_val_t res;
struct v7 *v7 = v7_create();

rcode = v7_exec(v7, "config = File.loadJSON('config.json')", NULL);
if (rcode != V7_OK) {
  fprintf(stderr, "exec error: %d\n", (int)rcode);
  goto clean;
}

rcode = v7_exec(v7, "config.devices[2].name", &res);   // Lookup value
if (rcode != V7_OK) {
  fprintf(stderr, "exec error: %d\n", (int)rcode);
  goto clean;
}

printf("Device2 name: ");
v7_println(v7, res);

clean:
v7_destroy(v7);
```

Note: JavaScript configuration may contain not only static data, but also some
dynamic logic in form of JavaScript function calls. That makes it possible to
write adaptive configuration and offload configuration logic to JavaScript.
