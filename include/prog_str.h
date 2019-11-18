#ifndef SPHUE_INCLUDE_PROG_STR_H_
#define SPHUE_INCLUDE_PROG_STR_H_

#include <avr/pgmspace.h>

#define READ_PROG_STR(progmem_str, dest_name) \
  char dest_name[sizeof(progmem_str)];\
  strcpy_P(dest_name, (PGM_P) progmem_str);

#endif //SPHUE_INCLUDE_PROG_STR_H_
