#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

int debug_vprintf(void (*putc)(char), const char *format, va_list args);

int debug_snprintf(char *buf, unsigned maxbuf, const char *format, ...)
    __attribute((format(printf, 3, 4)));

#ifdef __cplusplus
}
#endif

#endif  // MINIPRINTF_H