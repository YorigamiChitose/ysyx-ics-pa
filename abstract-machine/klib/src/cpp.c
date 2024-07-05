#include <am.h>
#include <klib.h>

#ifndef __ISA_NATIVE__

void __dso_handle() {
}

void __cxa_guard_acquire() {
}

void __cxa_guard_release() {
}

void __cxa_atexit() {
  assert(0);
}

uint64_t __lshrdi3(uint64_t a, unsigned int shift) {
    if (shift == 0) {
        return a;
    } else if (shift >= 64) {
        return 0;
    } else if (shift >= 32) {
        // Shift more than 32 but less than 64
        return (uint64_t)(a >> 32) >> (shift - 32);
    } else {
        // General case
        return (a >> shift) | ((uint64_t)((uint32_t)a >> (shift - 32)) << (32 - shift));
    }
}

int64_t __ashldi3(int64_t a, unsigned int shift) {
    if (shift == 0) {
        return a;
    } else if (shift >= 64) {
        return 0;
    } else if (shift >= 32) {
        // Shift more than 32 but less than 64
        return (int64_t)((uint64_t)(a << (shift - 32)) << 32);
    } else {
        // General case
        return (a << shift) | ((uint64_t)((uint32_t)(a >> (32 - shift))) << 32);
    }
}

#endif
