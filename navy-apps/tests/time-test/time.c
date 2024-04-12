#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  uint64_t last_time = NDL_GetTicks();
  // printf("%llu\n", last_time);
  while (1) {
    uint64_t time_ms = NDL_GetTicks();
    if(time_ms > last_time + 500) {
      // printf("%llu\n", time_ms);
      last_time = time_ms;
      printf("FUCK!\n");
    }
  }
  NDL_Quit();
  return 0;
}
