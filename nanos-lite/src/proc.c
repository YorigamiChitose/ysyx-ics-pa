#include "am.h"
#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
PCB *current = NULL;

void context_kload(PCB *pcb, void (*entry)(void*), int code) {
  Area kstack = {&(pcb->stack[0]), &(pcb->stack[STACK_SIZE - 1])};
  pcb->cp = kcontext(kstack, entry, (void *)code);
}

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%d' for the %dth time!", arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  char *const argv[] = {"/bin/pal", "--skip", "test", 0};
  char *const envp[] = {"HOME=/home", "PATH=/bin", 0};
  // context_kload(&pcb[0], hello_fun, 0);
  context_uload(&pcb[1], "/bin/pal", argv, envp);
  // context_kload(&pcb[1], hello_fun, 1);
  // context_kload(&pcb[2], hello_fun, 2);
  // context_kload(&pcb[3], hello_fun, 3);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/dummy");
  // naive_uload(NULL, "/bin/hello");
  // naive_uload(NULL, "/bin/time-test");
  // naive_uload(NULL, "/bin/file-test");
  // naive_uload(NULL, "/bin/event-test");
  // naive_uload(NULL, "/bin/bmp-test");
  // naive_uload(NULL, "/bin/nslider");
  // naive_uload(NULL, "/bin/menu");
  // naive_uload(NULL, "/bin/nterm");
  // naive_uload(NULL, "/bin/bird");
  // naive_uload(NULL, "/bin/pal");
  // naive_uload(NULL, "/bin/coremark");
  // naive_uload(NULL, "/bin/dhrystone");
  // naive_uload(NULL, "/bin/typing-game");
  // naive_uload(NULL, "/bin/fceux");
}

Context* schedule(Context *prev) {
  current->cp = prev;
  do {
    current = (&pcb[0] + ((current - &pcb[0] + 1) % MAX_NR_PROC));
  } while(!current->cp);
  return current->cp;
}
