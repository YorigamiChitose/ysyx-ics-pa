#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int cave_w = 0, cave_h = 0;
static int video_max_w = 0, video_max_h = 0;
static uint64_t NDL_start_time = 0;
static struct timeval tv;
static struct timezone tz;

uint32_t NDL_GetTicks() {
  gettimeofday(&tv, &tz);
  return tv.tv_usec / 1000 + tv.tv_sec * 1000 - NDL_start_time;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", 0, 0);
  if (fd == -1) {
    return 0;
  }
  int read_len = read(fd, buf, len);
  return read_len == -1 ? 0 : read_len;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (*w == 0 || *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  cave_h = *h;
  cave_w = *w;
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  x = (screen_w - cave_w) / 2 + x;
  y = (screen_h - cave_h) / 2 + y;
  int fd = open("/dev/fb", 0);
  for (int i = 0; i < h; i ++) {
    lseek(fd, (x + (y + i) * screen_w) * 4, SEEK_SET);
    write(fd, &(pixels[w * i]), w * 4);
  }
  close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  gettimeofday(&tv, &tz);
  NDL_start_time = tv.tv_usec / 1000 + tv.tv_sec * 1000;

  int fd = open("/proc/dispinfo", 0);
  uint8_t buf[128] = {0};
  read(fd, buf, 128);
  close(fd);
  sscanf((char *)buf, "WIDTH : %d\nHEIGHT : %d\n", &screen_w, &screen_h);
  return 0;
}

void NDL_Quit() {
}
