#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

struct GPU_config {
  uint32_t w;
  uint32_t h;
  uint32_t *fb;
  uint32_t size;
} config;

void __am_gpu_init() {
  config.h = inl(VGACTL_ADDR) & 0xffff;
  config.w = inl(VGACTL_ADDR) >> 16;
  config.size = config.w * config.h;
  config.fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (int i = 0; i < config.w * config.h; i ++) {
    config.fb[i] = 0x00000000;
  }
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = config.w, .height = config.h,
    .vmemsz = config.size
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t *pixels = ctl->pixels;
  for (uint32_t i = ctl->y; i < ctl->y + ctl->h; i++) {
    for (uint32_t j = ctl->x; j < ctl->x + ctl->w; j++) {
      config.fb[config.w * i + j] = pixels[ctl->w * (i - ctl->y) + (j - ctl->x)];
    }
  }


  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
