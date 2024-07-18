#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  yield();
  int i = 0;
  while (i != len) {
    putch(((char *)buf)[i++]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  yield();
  AM_INPUT_KEYBRD_T key = io_read(AM_INPUT_KEYBRD);
  if (key.keycode == AM_KEY_NONE) {
    return -1;
  }
  return snprintf(buf, len, "k%c %s\n", key.keydown ? 'd' : 'u', keyname[key.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T gpu = io_read(AM_GPU_CONFIG);
  return snprintf(buf, len, "WIDTH : %d\nHEIGHT : %d\n", gpu.width, gpu.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  yield();
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int x = offset / sizeof(uint32_t) % cfg.width;
  int y = offset / sizeof(uint32_t) / cfg.width;
  io_write(AM_GPU_FBDRAW, x, y, (void*)buf, len/sizeof(uint32_t), 1, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
