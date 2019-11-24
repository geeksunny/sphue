#include "prog_str.h"
#include <avr/pgmspace.h>

String read_prog_str(const char *progmem_str) {
  char dest[sizeof(progmem_str)];
  strcpy_P(dest, (PGM_P) progmem_str);
  return dest;
}

