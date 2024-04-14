include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/nemu.mk
CFLAGS  += -DISA_H=\"riscv/riscv.h\"
COMMON_CFLAGS += -march=rv32im_zicsr -mabi=ilp32   # overwrite
LDFLAGS       += -melf32lriscv                     # overwrite

AM_SRCS += riscv/nemu/start.S \
           riscv/nemu/cte.c \
           riscv/nemu/trap.S \
           riscv/nemu/vme.c

AM_SRCS += riscv/nemu/libgcc/div.S \
           riscv/nemu/libgcc/muldi3.S \
           riscv/nemu/libgcc/multi3.c \
           riscv/nemu/libgcc/ashldi3.c \
           riscv/nemu/libgcc/unused.c
