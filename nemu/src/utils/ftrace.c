#include <utils.h>
#include <common.h>

#ifdef CONFIG_FTRACE
enum {
  T_CALL,
  T_RET
};

struct FtraceNode ftraceList[CONFIG_FTRACE_BSIZE];
struct FtraceNode* ftraceHead = ftraceList;
struct FtraceNode* ftraceNow = ftraceList;
struct FuncInfo global = {
  "global",
  0x80000000,
  0
};


void init_ftrace_node(void) {
  struct FtraceNode* ftraceTemp = ftraceHead;
  ftraceTemp->srcFunc = &global;
  for (int i = 1; i < CONFIG_FTRACE_BSIZE; i++)
  {
    ftraceTemp->next = &ftraceList[i];
    ftraceTemp = ftraceTemp->next;
  }
  ftraceTemp->next = NULL;
}

void trace_func_call(paddr_t pcAddr, paddr_t dstAddr) {
  if (ftraceNow->next == NULL) {
    return;
  }
  for (int i = 0; i < allFucn.num; i++) {
    if (allFucn.funcInfo[i].addr == dstAddr) {
      ftraceNow->dstFunc = &(allFucn.funcInfo[i]);
      ftraceNow->next->srcFunc = &(allFucn.funcInfo[i]);
      ftraceNow->next->callDepth = ftraceNow->callDepth + 1;
      ftraceNow->type = T_CALL;
      ftraceNow->pcAddr = pcAddr;
      ftraceNow = ftraceNow->next;
      return;
    }
  }
  printf("not found function at " FMT_PADDR "\n", dstAddr);
  assert(0);
}

void trace_func_ret(paddr_t pcAddr, paddr_t dstAddr) {
  if (ftraceNow->next == NULL) {
    return;
  }
  for (int i = 0; i < allFucn.num; i++) {
    if ((allFucn.funcInfo[i].addr + allFucn.funcInfo[i].size) >= dstAddr && dstAddr >= allFucn.funcInfo[i].addr) {
      ftraceNow->dstFunc = &(allFucn.funcInfo[i]);
      ftraceNow->next->srcFunc = &(allFucn.funcInfo[i]);
      ftraceNow->callDepth -= 1;
      ftraceNow->next->callDepth = ftraceNow->callDepth;
      ftraceNow->type = T_RET;
      ftraceNow->pcAddr = pcAddr;
      ftraceNow = ftraceNow->next;
      return;
    }
  }
  printf("not found functin at %#010x\n", dstAddr);
  assert(0);
}

void trace_func_end(void) {
  if (ftraceNow->next == NULL) {
    return;
  }
  ftraceNow->callDepth -= 1;
  ftraceNow->dstFunc = &global;
  ftraceNow->type = T_RET;
  ftraceNow->pcAddr = 0x80000000;
  ftraceNow = ftraceNow->next;
}

void display_call(void) {
  struct FtraceNode* ftraceTemp = ftraceHead;
  printf("Call or ret from function \"global\" only means start program or end\n");
  while (ftraceTemp != ftraceNow) {
    printf(
      FMT_PADDR ": %*s%s [%s@" FMT_PADDR "] -> [%s@" FMT_PADDR "]\n",
      ftraceTemp->pcAddr, 
      ftraceTemp->callDepth, "", 
      ftraceTemp->type == T_CALL ? "call": "ret ", 
      ftraceTemp->srcFunc->name, 
      ftraceTemp->srcFunc->addr,
      ftraceTemp->dstFunc->name, 
      ftraceTemp->dstFunc->addr      
    );
    ftraceTemp = ftraceTemp->next;
  }
}
#endif