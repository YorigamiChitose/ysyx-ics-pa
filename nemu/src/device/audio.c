/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;
static bool audio_init_finish = false;
static int audio_sbuf_flag = 0;

static void audio_callback(void *userdata, uint8_t *stream, int len) {
  int len_ctrl = len;
  if(audio_base[reg_count] < len) {
    len_ctrl = audio_base[reg_count];
  }
  if(len_ctrl + audio_sbuf_flag < CONFIG_SB_SIZE) {
    memcpy(stream, sbuf + audio_sbuf_flag, len_ctrl);
    audio_sbuf_flag += len_ctrl;
  } else {
    memcpy(stream, sbuf + audio_sbuf_flag, CONFIG_SB_SIZE - audio_sbuf_flag);
    memcpy(stream + CONFIG_SB_SIZE - audio_sbuf_flag, sbuf, len_ctrl - (CONFIG_SB_SIZE - audio_sbuf_flag));
    audio_sbuf_flag = len_ctrl - (CONFIG_SB_SIZE - audio_sbuf_flag);
  }
  audio_base[reg_count] -= len_ctrl;
  if (len_ctrl < len) {
    memset(stream + len_ctrl, 0, len - len_ctrl);
  }
}

static void audio_init(void) {
  SDL_AudioSpec s = {};
  s.freq = audio_base[reg_freq];
  s.channels = audio_base[reg_channels];
  s.samples = audio_base[reg_samples];
  s.callback = audio_callback;
  s.userdata = NULL;
  audio_base[reg_count] = 0;
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_OpenAudio(&s, NULL);
  SDL_PauseAudio(0);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if (audio_base[reg_init] && !audio_init_finish && is_write) {
    audio_init_finish = true;
    audio_init();
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
