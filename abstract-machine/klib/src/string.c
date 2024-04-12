#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	assert(s != NULL);
	size_t len = 0;
	while(s[len] != '\0') {++len;}
	return len;
}

char *strcpy(char *dst, const char *src) {
  assert(src != NULL || dst != NULL);
  char *ret = dst;
  while ((*dst++ = *src++) != '\0') {};
	return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  assert(src != NULL || dst != NULL);
  char* tmp = dst;
	while (n && (*dst++ = *src++)) {n--;}
	if(n){while (--n){*dst++ = '\0';}}
	return(tmp);
}

char *strcat(char *dst, const char *src) {
  assert(src != NULL || dst != NULL);
  char *tmp = dst;
  while (*dst) {dst++;}
  while ((*dst++ = *src++) != '\0');
  return tmp;
}

int strcmp(const char *s1, const char *s2) {
  assert(s1 != NULL || s2 != NULL);
  int ret = 0 ;
  while(!(ret = *(unsigned char *)s1 - *(unsigned char *)s2) && *s2) {++s1, ++s2;}  
  if (ret == 0) {return 0;}
  return ret > 0 ? 1 : -1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  assert(s1 != NULL || s2 != NULL);
  int ret = 0 ;
  while(!(ret = *(unsigned char *)s1 - *(unsigned char *)s2) && *s2 && (n - 1)) {++s1, ++s2, --n;}  
  if (ret == 0) {return 0;}
  return ret > 0 ? 1 : -1;
}

void *memset(void *s, int c, size_t n) {
  assert(s != NULL);
  const unsigned char ch = c;
  unsigned char* p_ch;
  for (p_ch = s; 0 < n; ++p_ch, --n) {*p_ch = ch;}
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  assert(src != NULL || dst != NULL);
  void* ret = dst;
	if (dst < src)
	{
		while (n--)
		{
			*(char*)dst = *(char*)src;
			dst = (char*)dst + 1;
			src = (char*)src + 1;
		}
	}
	else
	{
		while (n--) {*((char*)dst + n) = *((char*)src + n);}
	}
	return ret;
}

void *memcpy(void *out, const void *in, size_t n) {
  assert(out != NULL || in != NULL);
  void* ret = out;
  while (n--) {
    *(char*)out = *(char*)in;
    out = (char*)out + 1;
    in = (char*)in + 1;
  }
  return ret;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  assert(s1 != NULL || s2 != NULL);
  while (n--) {
    if (*(char*)s1 == *(char*)s2) {
      s1 = (char*)s1 + 1;
      s2 = (char*)s2 + 1;
    }
    else {return (*(char*)s1 - *(char*)s2);}
  }
  return 0;
}

#endif
