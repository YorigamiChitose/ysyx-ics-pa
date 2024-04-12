#include <am.h>
#include <stdint.h>
#include <string.h>
#include "NDL.h"

#define SYNC_ADDR (VGACTL_ADDR + 4)

struct GPU_config {
  uint32_t w;
  uint32_t h;
  uint32_t size;
} config = {.h = 300, .w = 400, .size = 400 * 300};

uint32_t nop[400 * 300];
void __am_gpu_init() {
  NDL_OpenCanvas((int *)&config.w, (int *)&config.h);
  NDL_DrawRect(nop, 0, 0, config.w, config.h);
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
  NDL_DrawRect(pixels, ctl->x, ctl->y, ctl->w, ctl->h);


  // if (ctl->sync) {
  //   outl(SYNC_ADDR, 1);
  // }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
