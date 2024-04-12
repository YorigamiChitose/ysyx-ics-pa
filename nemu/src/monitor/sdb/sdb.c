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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;
  HIST_ENTRY *last_line = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  } else if (line_read && (history_length > 0)) {
    last_line = history_get(history_length);
    free(line_read);
    line_read = malloc((strlen(last_line->line) + 1) * sizeof(char));
    memcpy(line_read, last_line->line, (strlen(last_line->line) + 1) * sizeof(char));
  }

  return line_read;
}

static int cmd_help(char *args);
static int cmd_c(char *args);
static int cmd_q(char *args);
// static int cmd_si(char *args);
// static int cmd_info(char *args);
// static int cmd_x(char *args);
// static int cmd_p(char *args);
// static int cmd_w(char *args);
// static int cmd_d(char *args);
// static int cmd_trace(char *args);
// static int cmd_detach(char *args);
// static int cmd_attach(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  {"help",   "Usage: help\t[str]\t\t - Display information about all supported commands.", cmd_help},
  {"c",      "Usage: c\t\t\t - Continue the execution of the program.", cmd_c},
  {"q",      "Usage: q\t\t\t - Exit NEMU.", cmd_q},
  // {"si",     "Usage: si\t[int]\t\t - Run step by step.", cmd_si},
  // {"info",   "Usage: info\tr | w\t\t - Get the info.", cmd_info},
  // {"x",      "Usage: x\t[int] [str]\t - Scan the memory.", cmd_x},
  // {"p",      "Usage: p\t[str]\t\t - Calculate.", cmd_p},
  // {"w",      "Usage: w\t[str]\t\t - Set a new watchpoint.", cmd_w},
  // {"d",      "Usage: d\t[int]\t\t - Delete a watchpoint.", cmd_d},
  // {"trace",  "Usage: trace\t[str]\t\t - Check the inst.", cmd_trace},
  // {"detach", "Usage: detach\t\t\t - Disable difftest.", cmd_detach},
  // {"attach", "Usage: attach\t\t\t - Enable difftest.", cmd_attach},

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  if (nemu_state.state == NEMU_ABORT) {
    nemu_state.halt_ret = 1;
  }
  if (nemu_state.halt_ret == 0) {
    nemu_state.state = NEMU_QUIT;
  }
  return -1;
}

static int cmd_help(char *args) {
  char *arg = strtok(NULL, " ");
  int i;
  if (arg == NULL) {
    for (i = 0; i < NR_CMD; i ++) {
      printf("%-6s- %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%-6s- %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Error! Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
