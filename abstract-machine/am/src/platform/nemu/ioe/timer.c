#include <am.h>
#include <nemu.h>

static AM_TIMER_RTC_T day_time;

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint32_t high = inl(RTC_ADDR+4);
  uint32_t low = inl(RTC_ADDR);
  uptime->us = (uint64_t)low + (((uint64_t)high) << 32);
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = day_time.second;
  rtc->minute = day_time.minute;
  rtc->hour   = day_time.hour;
  rtc->day    = day_time.day;
  rtc->month  = day_time.month;
  rtc->year   = day_time.year;
}
