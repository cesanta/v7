---
title: Usage
---

V7 is deployed in two files `v7.h` and `v7.c`.  `v7.h` defines V7 C/C++
embedding API. `v7.c` implements JavaScript engine.
In order to embed V7 in your program,

- Copy `v7.c` and `v7.h` to your project
- Write code that uses V7 API, e.g. in `my_app.c`
- Compile application: `$ cc my_app.c mongoose.c`

```c
/* Include V7 API definitions */
#include "v7.h"

int main(int argc, char *argv[]) {
  const char *js_code = argc > 1 ? argv[1] : "";
  v7_val_t exec_result;

  /* Create V7 instance */
  struct v7 *v7 = v7_create();

  /* Execute a string given in a command line argument */
  v7_exec(v7, js_code, &exec_result);

  /* Destroy V7 instance */
  v7_destroy(v7);
  return 0;
}
```

Running the application above gives:

```
    ./my_app "var a = [1,2,3].splice(2); print(a)"
```
