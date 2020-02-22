#ifndef BEARWASM_LIBC_H
#define BEARWASM_LIBC_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

namespace bearwasm {

void vsnprintf(char *buf, size_t len, const char *fmt, va_list arg);
} /* namespace bearwasm */

#endif
