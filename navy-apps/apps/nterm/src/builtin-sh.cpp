#include <nterm.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <SDL.h>

extern char ** global_envp;
char handle_key(SDL_Event *ev);

static int cmd_help(char *args);
// static int cmd_echo(char *args);
static int cmd_exit(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  {"help", "Usage: help \033[35m[str]\033[0m - Help information", cmd_help},
  // {"echo", "Usage: echo \033[35m[str]\033[0m - Display string"  , cmd_echo},
  {"exit", "Usage: echo \033[35m     \033[0m - Exit"  , cmd_exit},
};

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  char *arg = strtok(NULL, " ");
  int i;
  if (arg == NULL) {
    for (i = 0; i < NR_CMD; i ++) {
      sh_printf("%-6s- %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        sh_printf("%-6s- %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    sh_printf("Error! Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_echo(char *args) {
  if (args != NULL) {
    sh_printf("%s\n", args);
  }
  return 0;
}

static int cmd_exit(char *args) {
  exit(0);
}

static void sh_handle_cmd(const char *cmd) {
  char cmd_temp[128] = {};
  strcpy(cmd_temp, cmd);
  char *arg_end = cmd_temp + strlen(cmd_temp) - 1;
  *arg_end = 0;
  char *cmd_start = strtok(cmd_temp, " ");

  if (cmd_start == NULL) { return; }
  char *arg_start = cmd_start + strlen(cmd_start) + 1;
  if (arg_start >= arg_end) {
    arg_start = NULL;
  }
  for (int i = 0; i < NR_CMD; i ++) {
    if (strcmp(cmd_start, cmd_table[i].name) == 0) {
      if (cmd_table[i].handler(arg_start) < 0) { return; }
      break;
    }
  }
  char *argv[10] = {cmd_start, NULL};
  char *token = strtok((char *)arg_start, " ");
  int argc = 1;
  while (token != NULL && argc < 10) {
    argv[argc++] = token;
    token = strtok(NULL, " ");
  }
  if (fopen(cmd_start, "r") != NULL) {
    if (execve(cmd_start, argv, global_envp) == -1) {
      exit(EXIT_FAILURE);
    }
  }
  char path[128] = "";
  char envc = 0;
  while (strncmp("PATH=", global_envp[envc], 5)) {
    envc ++;
  }
  strncpy(path, &(global_envp[envc][5]), 128);
  char *prefix = strtok(path, ";");
  char full_path[128] = "";
  while (prefix != NULL) {
    full_path[0] = '\0';
    strncpy(full_path, prefix, 128);
    strcat(full_path, cmd_start);
    FILE *fd = fopen(full_path, "r");
    if (fd != NULL) {
      if (execve(full_path, argv, global_envp) == -1) {
        exit(EXIT_FAILURE);
      }
      fclose(fd);
    }
    prefix = strtok(NULL, ";");
  }
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);

          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
