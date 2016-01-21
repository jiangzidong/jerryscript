#ifndef JERRY_LIBC_STRING_H
#define JERRY_LIBC_STRING_H

#include <stddef.h>



extern  int    memcmp (const void *s1, const void *s2, size_t n);
extern  void*  memcpy (void *dest, const void *src, size_t n);
extern  void*  memset (void *s, int c, size_t n);
extern  int    strcmp (const char *s1, const char *s2);
extern  size_t strlen (const char *s);
extern  void*  memmove (void *dest, const void *src, size_t n);
extern  int    strncmp (const char *s1, const char *s2, size_t n);
extern  char*  strncpy (char *dest, const char *src, size_t n);

#endif /* !JERRY_LIBC_STRING_H */