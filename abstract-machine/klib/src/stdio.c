#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define PRINTF_BUF 4096
char pf_buf[PRINTF_BUF];
int vsprintf(char *out, const char *fmt, va_list vp);
int vsnprintf(char *out, size_t n, const char *fmt, va_list ap);
static const char num_ch[16] = "0123456789ABCDEF";

static char* sput_ch(char* out, char ch) {
  *out++ = ch;
  return out;
}

static char* sput_str(char* out, char* str) {
  int str_len = strlen(str);
  for (int n = 0; n < str_len; n++) {
    out = sput_ch(out, str[n]);
  }
  return out;
}

static char* sput_num(char* out, int num, int dnum) {
  if (num == 0) {
    return out;
  }
  out = sput_num(out, num / dnum, dnum);
  out = sput_ch(out, num_ch[num % dnum]);
  return out;
}

static char* sput_unum(char* out, unsigned int num, unsigned int dnum) {
  if (num == 0) {
    return out;
  }
  out = sput_num(out, num / dnum, dnum);
  out = sput_ch(out, num_ch[num % dnum]);
  return out;
}

static char* sput_hexnum(char* out, int hexnum) {
  if (hexnum < 0) {
    out = sput_ch(out, '-');
    hexnum = -hexnum;
  }
  else if (hexnum == 0) {
    out = sput_ch(out, '0');
  }
  out = sput_num(out, hexnum, 16);
  return out;
}

static char* sput_uhexnum(char* out, int hexnum) {
  if (hexnum == 0) {
    out = sput_ch(out, '0');
  }
  out = sput_unum(out, hexnum, 16);
  return out;
}

static char* sput_decnum(char* out, int decnum) {
  if (decnum < 0) {
    out = sput_ch(out, '-');
    decnum = -decnum;
  }
  else if (decnum == 0) {
    out = sput_ch(out, '0');
  }
  out = sput_num(out, decnum, 10);
  return out;
}

static char* sput_udecnum(char* out, unsigned int decnum) {
  if (decnum == 0) {
    out = sput_ch(out, '0');
  }
  out = sput_unum(out, decnum, 10);
  return out;
}

int printf(const char *fmt, ...) {
  va_list vp;
  va_start(vp, fmt);
  int put_count = vsprintf(pf_buf, fmt, vp);
  putstr(pf_buf);
  return put_count;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, -1, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list vp;
  va_start(vp, fmt);
  
  return vsprintf(out, fmt, vp);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list vp;
  va_start(vp, fmt);
  
  return vsnprintf(out, n, fmt, vp);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  const char *pfmt = fmt;
  char *out_head = out;
  char flag = false;
  while(*pfmt) {
    if (out - out_head == n) {
      return n;
    }
    switch (*pfmt) {
      case '%': 
        flag = true;
        while (flag) {
          switch (*(pfmt + 1)) {
            case 'c':
              out = sput_ch(out, va_arg(ap, int));
              ++pfmt; flag = false;
              break;
            case 'p':
              out = sput_uhexnum(out, va_arg(ap, unsigned int));
              ++pfmt; flag = false;
              break;
            case 'x':
              out = sput_hexnum(out, va_arg(ap, int));
              ++pfmt; flag = false;
              break;
            case 'd': 
              out = sput_decnum(out, va_arg(ap, int));
              ++pfmt; flag = false;
              break;
            case 'u':
              out = sput_udecnum(out, va_arg(ap, unsigned int));
              ++pfmt; flag = false;
              break;
            case 's':
              out = sput_str(out, va_arg(ap, char*));
              ++pfmt; flag = false;
              break;
            default:
              ++pfmt;
              continue;
          }
        }
        break;
      default: out = sput_ch(out, *pfmt); break;
    }
    ++pfmt;
  }
  sput_ch(out, '\0');
  return out - out_head;
}

#endif
