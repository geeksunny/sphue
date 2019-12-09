#ifndef SPHUE_INCLUDE_PGMSTRINGTOOLS_HPP_
#define SPHUE_INCLUDE_PGMSTRINGTOOLS_HPP_

#include <cstring>
#include "prog_str.h"

constexpr int max(int a) {
  return a;
}

template<typename... Ints>
constexpr int max(int a, int b, Ints... ints) {
  return max(((a > b) ? a : b), ints...);
}

template<typename... StringLengths>
constexpr int get_buffer_size(StringLengths... string_lengths) {
  return max(string_lengths...);
}

#define SL(var) sizeof(var)

#define PGM_STR_EQ(pgm_str, c_str, pgms_buffer) (std::strcmp(c_str, read_prog_str(pgm_str, pgms_buffer)) == 0)

#define STR_EQ_INIT(cstr_ptr, first_pgmstr_length, ...)\
  const char *the_str = cstr_ptr;\
  char the_buf[get_buffer_size(first_pgmstr_length, ##__VA_ARGS__)];

#define STR_EQ_RET(progmem_str, return_value) if (PGM_STR_EQ(progmem_str, the_str, the_buf)) { return return_value; }

#define STR_EQ_DO(progmem_str, code_block) if (PGM_STR_EQ(progmem_str, the_str, the_buf)) { code_block }

#endif //SPHUE_INCLUDE_PGMSTRINGTOOLS_HPP_
