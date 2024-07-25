#include "memory.h"
#include "sys/types.h"
#include <proc.h>
#include <elf.h>
#include <common.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  int fd = fs_open(filename, 0, 0);
  fs_lseek(fd, 0, SEEK_SET);
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  assert(!strncmp((char*)ehdr.e_ident, ELFMAG, SELFMAG));

  Elf_Phdr phdr[ehdr.e_phnum];
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read(fd, phdr, sizeof(Elf_Phdr)*ehdr.e_phnum);

  for (int i = 0; i < ehdr.e_phnum; i++) {
    if (phdr[i].p_type == PT_LOAD) {
      fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
      fs_read(fd, (void*)phdr[i].p_vaddr, phdr[i].p_filesz);
      memset((void*)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
    }
  }
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  Area kstack = {&(pcb->stack[0]), &(pcb->stack[STACK_SIZE - 1])};
  uintptr_t entry = loader(pcb, filename);

  uint argc = 0;
  uint envc = 0;
  while (argv[argc] != NULL) { argc ++; }
  while (envp[envc] != NULL) { envc ++; }
  uint str_area_size = 0;
  for (uint i = 0; i < argc; i ++) {
    str_area_size += strlen(argv[i]) + 1;
  }
  for (uint i = 0; i < envc; i ++) {
    str_area_size += strlen(envp[i]) + 1;
  }
  uintptr_t *str_pointer = (uintptr_t *)(((uintptr_t)new_page(1) - str_area_size) & ~(sizeof(uintptr_t) - 1));
  uintptr_t *arr_pointer = str_pointer - (1 + 1 * 2 + argc + envc);
  arr_pointer[0] = (uintptr_t)argc;
  for (uint i = 0; i < argc; i ++) {
    uint len = strlen(argv[i]) + 1;
    arr_pointer[i + 1] = (uintptr_t)str_pointer;
    strcpy((char *)str_pointer, argv[i]);
    str_pointer = (uintptr_t *)((char *)str_pointer + len);
  }
  arr_pointer[argc + 1] = (uintptr_t)NULL;
  for (uint i = 0; i < envc; i ++) {
    uint len = strlen(envp[i]) + 1;
    arr_pointer[argc + 2 + i] = (uintptr_t)str_pointer;
    strcpy((char *)str_pointer, envp[i]);
    str_pointer = (uintptr_t *)((char *)str_pointer + len);
  }
  arr_pointer[argc + 2 + envc] = (uintptr_t)NULL;
  pcb->cp = ucontext(NULL, kstack, (void *)entry);
  pcb->cp->gpr[10] = (uintptr_t)arr_pointer;
}