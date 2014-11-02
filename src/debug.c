#include <stdio.h>
#include <stdarg.h>

#include "debug.h"

int debug_fprintf(FILE * fp, const char * fmt, ...)
{
  #ifndef NDEBUG_MESSAGES
  int chars_written = 0;
  va_list args;
  
  va_start(args, fmt);
  chars_written = vfprintf(fp, fmt, args);
  va_end(args);
  return chars_written;
  #else
  return 0;
  #endif
}
