#include <am.h>
#include "NDL.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define KEYDOWN_MASK 0x8000
#define NAMEINIT(key)  [ AM_KEY_##key ] = #key,
static const char *names[] = {
  AM_KEYS(NAMEINIT)
};
#define KEYNUM sizeof(names) / sizeof(names[0])

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  char buf[64];
  if (NDL_PollEvent(buf, sizeof(buf))) {
    if (buf[0] == 'k') {
      char key_type;
      char key_buf[32];
      assert(sscanf(buf, "k%c %s", &key_type, key_buf) == 2);
      switch (key_type) {
        case 'u': kbd->keydown = false; break;
        case 'd': kbd->keydown = true; break;
        default: assert(0); break;
      }
      for (int i = 1; i < KEYNUM; i ++) {
        if (strcmp(names[i], key_buf) == 0) {
          kbd->keycode = i;
        }
      }
    }
  }
}
