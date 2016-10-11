#include "mbed.h"

#include "v7.h"

DigitalOut led_green(LED1);

enum v7_err js_set_led(struct v7 *v7, v7_val_t *res) {
  led_green = !v7_get_bool(v7, v7_arg(v7, 0));
  return V7_OK;
}

enum v7_err js_wait(struct v7 *v7, v7_val_t *res) {
  wait(v7_get_double(v7, v7_arg(v7, 0)));
  return V7_OK;
}

int main() {
  struct v7 *v7 = v7_create();

  v7_set_method(v7, v7_get_global(v7), "setLed", &js_set_led);
  v7_set_method(v7, v7_get_global(v7), "wait", &js_wait);

  v7_exec(v7,
          "while(true) {" \
          "  setLed(false);" \
          "  wait(0.5); " \
          "  setLed(true);" \
          "  wait(0.5); " \
          "}",
          NULL);
}
