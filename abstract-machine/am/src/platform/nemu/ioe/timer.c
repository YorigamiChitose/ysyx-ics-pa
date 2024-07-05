#include <am.h>
#include <nemu.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint32_t high = inl(RTC_ADDR+4);
  uint32_t low = inl(RTC_ADDR);
  uptime->us = (uint64_t)low + (((uint64_t)high) << 32);
}

int is_leap_year(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int days_in_month(int year, int month) {
  static const int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if (month == 2 && is_leap_year(year)) {
    return 29;
  }
  return days[month - 1];
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  uint32_t high         = inl(RTC_ADDR+4);
  uint32_t low          = inl(RTC_ADDR);
  uint64_t total_second = ((uint64_t)low + (((uint64_t)high) << 32)) / 1000000;

  uint32_t days          = total_second / 86400;
  uint32_t second_in_day = total_second % 86400;

  rtc->year = 1970;

  while(true) {
    int days_in_year = is_leap_year(rtc->year) ? 366 : 365;
    if (days >= days_in_year) {
      days -= days_in_year;
      rtc->year++;
    } else {
      break;
    }
  }

  rtc->month = 1;

  while (1) {
    int dim = days_in_month(rtc->year, rtc->month);
    if (days >= dim) {
      days -= dim;
      rtc->month++;
    } else {
      break;
    }
  }

  rtc->day    = days + 1;

  rtc->hour   = second_in_day / 3600;
  second_in_day %= 3600;
  rtc->minute = second_in_day / 60;
  rtc->second = second_in_day % 60;
}
