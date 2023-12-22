#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void lc_DebugPrint(const char_t format[], ...)
{
   va_list args;
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
}