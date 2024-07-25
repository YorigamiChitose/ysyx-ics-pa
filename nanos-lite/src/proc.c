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
  char *const argv[] = {"/bin/nterm", 0};
  char *const envp[] = {"PATH=/bin/;/usr/bin/"};
  // context_kload(&pcb[0], hello_fun, 0);
  context_uload(&pcb[0], "/bin/nterm", argv, envp);

  switch_boot_pcb();

  Log("Initializing processes...");
}

Context* schedule(Context *prev) {
  current->cp = prev;
  do {
    current = (&pcb[0] + ((current - &pcb[0] + 1) % MAX_NR_PROC));
  } while(!current->cp);
  return current->cp;
}
