#ifndef SPHUE_INCLUDE_PROG_STR_H_
#define SPHUE_INCLUDE_PROG_STR_H_

#include <WString.h>

String read_prog_str(const char *progmem_str);
char *read_prog_str(const char *progmem_str, char *dest);

#endif //SPHUE_INCLUDE_PROG_STR_H_
