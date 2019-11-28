#ifndef SPHUE_INCLUDE_ENUMTOOLS_HPP_
#define SPHUE_INCLUDE_ENUMTOOLS_HPP_

#include <cstring>
#include "prog_str.h"

// pgm str -> enum

template<typename PgmStrType, typename EnumType>
constexpr int largest_pgm_str_length_s2e(PgmStrType progmem_str, EnumType enum_value) {
  return sizeof(progmem_str);
}

template<typename PgmStrType, typename EnumType, typename... PSTsETs>
constexpr int largest_pgm_str_length_s2e(PgmStrType progmem_str, EnumType enum_value, PSTsETs... pgm_strs_and_enums) {
  return (sizeof(progmem_str) > largest_pgm_str_length_s2e(progmem_str, enum_value, pgm_strs_and_enums...))
         ? sizeof(progmem_str) : largest_pgm_str_length_s2e(progmem_str, enum_value, pgm_strs_and_enums...);
}

template<typename EnumType, typename PgmStrType>
EnumType pgmStringToEnum(const char *str_value,
                         char *str_buffer,
                         EnumType default_value,
                         PgmStrType last_mapped_pgm_str,
                         EnumType last_mapped_enum) {
  if (std::strcmp(str_value, read_prog_str(last_mapped_pgm_str, str_buffer)) == 0) {
    return last_mapped_enum;
  } else {
    return default_value;
  }
}

template<typename EnumType, typename PgmStrType, typename... PSTsETs>
EnumType pgmStringToEnum(const char *str_value,
                         char *str_buffer,
                         EnumType default_value,
                         PgmStrType first_mapped_pgm_str,
                         EnumType first_mapped_enum,
                         PSTsETs... pgm_strs_and_enums) {
  if (std::strcmp(str_value, read_prog_str(first_mapped_pgm_str, str_buffer)) == 0) {
    return first_mapped_enum;
  } else {
    return pgmStringToEnum(str_value, str_buffer, default_value, pgm_strs_and_enums...);
  }
}

#define PGM_STRING_TO_ENUM(str_value, enum_dest_name, default_value, first_mapped_pgm_str, first_mapped_enum, ...)\
  char buf[largest_pgm_str_length_s2e(first_mapped_pgm_str, first_mapped_enum, __VA_ARGS__)];\
  auto enum_dest_name = pgmStringToEnum(str_value,\
                                        buf,\
                                        default_value,\
                                        first_mapped_pgm_str,\
                                        first_mapped_enum,\
                                        __VA_ARGS__);

#define RETURN_PGM_STRING_TO_ENUM(str_value, default_value, first_mapped_pgm_str, first_mapped_enum, ...)\
  PGM_STRING_TO_ENUM(str_value, result, default_value, first_mapped_pgm_str, first_mapped_enum, __VA_ARGS__);\
  return result;

// enum -> pgm str

template<typename EnumType, typename PgmStrType>
constexpr int largest_pgm_str_length_e2s(EnumType enum_value, PgmStrType progmem_str) {
  return sizeof(progmem_str);
}

template<typename EnumType, typename PgmStrType, typename... ETsPSTs>
constexpr int largest_pgm_str_length_e2s(EnumType enum_value, PgmStrType progmem_str, ETsPSTs... enums_and_pgm_strs) {
  return (sizeof(progmem_str) > largest_pgm_str_length_e2s(progmem_str, enum_value, enums_and_pgm_strs...))
         ? sizeof(progmem_str) : largest_pgm_str_length_e2s(progmem_str, enum_value, enums_and_pgm_strs...);
}

template<typename EnumType, typename PgmStrType>
const char *enumToPgmString(EnumType enum_value,
                            char *str_buffer,
                            PgmStrType default_value,
                            EnumType last_mapped_enum,
                            PgmStrType last_mapped_pgm_str) {
  if (enum_value == last_mapped_enum) {
    return read_prog_str(last_mapped_pgm_str, str_buffer);
  } else {
    return read_prog_str(default_value, str_buffer);
  }
}

template<typename EnumType, typename PgmStrType, typename... ETsPSTs>
const char *enumToPgmString(EnumType enum_value,
                            char *str_buffer,
                            PgmStrType default_value,
                            EnumType first_mapped_enum,
                            PgmStrType first_mapped_pgm_str,
                            ETsPSTs... enums_and_pgm_strs) {
  if (enum_value == first_mapped_enum) {
    return read_prog_str(first_mapped_pgm_str, str_buffer);
  } else {
    return enumToPgmString(enum_value, str_buffer, default_value, enums_and_pgm_strs...);
  }
}

#define ENUM_TO_PGM_STRING(enum_value, string_dest_name, default_value, first_mapped_enum, first_mapped_pgm_str, ...)\
  char string_dest_name[largest_pgm_str_length_e2s(first_mapped_enum, first_mapped_pgm_str, __VA_ARGS__)];\
  enumToPgmString(enum_value, string_dest_name, default_value, first_mapped_enum, first_mapped_pgm_str, __VA_ARGS__);

#define RETURN_ENUM_TO_PGM_STRING(enum_value, default_value, first_mapped_enum, first_mapped_pgm_str, ...)\
  ENUM_TO_PGM_STRING(enum_value, result, default_value, first_mapped_enum, first_mapped_pgm_str, __VA_ARGS__)\
  return result;

#endif //SPHUE_INCLUDE_ENUMTOOLS_HPP_
