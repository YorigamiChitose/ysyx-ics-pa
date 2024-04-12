#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

#define KEYNUM sizeof(keyname) / sizeof(keyname[0])
uint8_t key_status[KEYNUM] = {0};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  if (NDL_PollEvent(buf, sizeof(buf))) {
    if (buf[0] == 'k') {
      char key_type;
      char key_buf[32];
      assert(sscanf(buf, "k%c %s", &key_type, key_buf) == 2);
      switch (key_type) {
        case 'u': ev->type = SDL_KEYUP; break;
        case 'd': ev->type = SDL_KEYDOWN; break;
        default: assert(0); break;
      }
      for (int i = 1; i < KEYNUM; i ++) {
        if (strcmp(keyname[i], key_buf) == 0) {
          ev->key.keysym.sym = i;
          key_status[i] = !ev->type;
          return 1;
        }
      }
    }
  }
  return 0;
}

int SDL_WaitEvent(SDL_Event *ev) {
  while (true) {
    if (SDL_PollEvent(ev)) {
      return 1;
    }
  }
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return key_status;
}
