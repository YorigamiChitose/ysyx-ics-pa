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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

extern const char *regs[];

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if (ref_r->pc != cpu.pc) {
    Log("Different values of the PC! REF: " FMT_PADDR " DUT: " FMT_PADDR , ref_r->pc, cpu.pc);
    return false;
  }
  for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
    if (ref_r->gpr[i] != gpr(i)) {
      Log("Different values of reg %s! REF: " FMT_WORD " DUT: " FMT_WORD, regs[i], ref_r->gpr[i], gpr(i));
      return false;
    }
  }
  if (ref_r->csr.mepc != cpu.csr.mepc) {
    Log("Different values of csr mepc! REF: " FMT_WORD " DUT: " FMT_WORD, ref_r->csr.mepc, cpu.csr.mepc);
    return false;
  }
  if (ref_r->csr.mcause != cpu.csr.mcause) {
    Log("Different values of csr mcause! REF: " FMT_WORD " DUT: " FMT_WORD, ref_r->csr.mcause, cpu.csr.mcause);
    return false;
  }
  if (ref_r->csr.mstatus != cpu.csr.mstatus) {
    Log("Different values of csr mstatus! REF: " FMT_WORD " DUT: " FMT_WORD, ref_r->csr.mstatus, cpu.csr.mstatus);
    return false;
  }
  if (ref_r->csr.mtvec != cpu.csr.mtvec) {
    Log("Different values of csr mtvec! REF: " FMT_WORD " DUT: " FMT_WORD, ref_r->csr.mtvec, cpu.csr.mtvec);
    return false;
  }
  return true;
}

void isa_difftest_attach() {
}
