#include <common.h>
#include <fs.h>
#include <sys/time.h>
#include <proc.h>
#include "syscall.h"

void naive_uload(PCB *pcb, const char *filename);

void trace_syscall(uintptr_t code) {
  switch (code) {
    case -1: Debug("System call: yield"); break;
    case SYS_exit: Debug("System call: SYS_exit"); break;
    case SYS_yield: Debug("System call: SYS_yield"); break;
    case SYS_open: Debug("System call: SYS_open"); break;
    case SYS_read: Debug("System call: SYS_read"); break;
    case SYS_write: Debug("System call: SYS_write"); break;
    case SYS_close: Debug("System call: SYS_close"); break;
    case SYS_lseek: Debug("System call: SYS_lseek"); break;
    case SYS_brk: Debug("System call: SYS_brk"); break;
    case SYS_gettimeofday: Debug("System call: SYS_gettimeofday"); break;
    default: Debug("System call: Error"); break;
  }
}

intptr_t mybrk(uintptr_t increment) {
  return 0;
}

int mygettimeofday(struct timeval *tv, struct timezone *tz) {
  AM_TIMER_UPTIME_T t = io_read(AM_TIMER_UPTIME);
  if (tv) {
    tv->tv_usec = t.us % (1000 * 1000);
    tv->tv_sec = t.us / (1000 * 1000);
  }
  if (tz) {
    tz->tz_minuteswest = -480;
    tz->tz_dsttime = 0;
  }
  return 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  trace_syscall(a[0]);

  switch (a[0]) {
    case SYS_exit: halt(a[1]); break;
    // case SYS_exit: naive_uload(NULL, "/bin/menu"); break;
    case SYS_yield: Log("SYS_yield"); break;
    case SYS_open: c->GPRx = fs_open((const char *)a[1], a[2], a[3]); break;
    case SYS_read: c->GPRx = fs_read(a[1], (void *)a[2], a[3]); break;
    case SYS_write: c->GPRx = fs_write(a[1], (const char *)a[2], a[3]); break;
    case SYS_close: c->GPRx = fs_close(a[1]); break;
    case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
    case SYS_brk: c->GPRx = mybrk(a[1]); break;
    case SYS_execve: naive_uload(NULL, (const char *)a[1]); break;
    case SYS_gettimeofday: c->GPRx = mygettimeofday((struct timeval *)a[1], (struct timezone *)a[2]); break;
    default: panic("Unhandled syscall ID = %d", a[0]); break;
  }
}
