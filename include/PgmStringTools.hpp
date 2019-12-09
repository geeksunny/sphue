#ifndef SPHUE_INCLUDE_PGMSTRINGTOOLS_HPP_
#define SPHUE_INCLUDE_PGMSTRINGTOOLS_HPP_

#include <cstring>
#include "prog_str.h"

#define PGM_STR_EQ(pgm_str, c_str, c_str_len, pgms_buffer)\
  (c_str_len == sizeof(pgm_str) && std::strcmp(c_str, read_prog_str(pgm_str, pgms_buffer)) == 0)

#define STR_EQ_INIT(cstr_ptr)\
  const char *the_str = cstr_ptr;\
  unsigned int the_str_len = std::strlen(the_str) + 1;\
  char the_buf[the_str_len];

#define STR_EQ_RET(progmem_str, return_value)\
  if (PGM_STR_EQ(progmem_str, the_str, the_str_len, the_buf)) { return return_value; }

#define STR_EQ_DO(progmem_str, code_block)\
  if (PGM_STR_EQ(progmem_str, the_str, the_str_len, the_buf)) { code_block }

#endif //SPHUE_INCLUDE_PGMSTRINGTOOLS_HPP_
