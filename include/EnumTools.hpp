#ifndef SPHUE_INCLUDE_ENUMTOOLS_HPP_
#define SPHUE_INCLUDE_ENUMTOOLS_HPP_

#include <array>
#include <cstring>
#include "prog_str.h"

// Enum<->ProgmemString Mapping facilities

template<typename Enum>
struct EnumMapping {
  Enum enum_value;
  const char *pgm_string;
  int length;

  constexpr EnumMapping(Enum enum_value, const char *pgm_string, int length) :
      enum_value(enum_value), pgm_string(pgm_string), length(length) {}
};

template<typename Enum, std::size_t N>
constexpr std::array<EnumMapping<Enum>, N> _make_mapping(std::initializer_list<EnumMapping<Enum>> mappings) {
  return std::array<EnumMapping<Enum>, N>(mappings);
}

template<typename Enum, typename... Mappings>
constexpr std::array<EnumMapping<Enum>, sizeof...(Mappings)> make_mapping(Mappings... mappings) {
  return {{mappings...}};
}

#define PGM_STR_AND_SIZE(pgm_string) pgm_string, sizeof(pgm_string)

#define MAPPING(enum_value, pgm_string) EnumMapping<decltype(enum_value)>{ enum_value, PGM_STR_AND_SIZE(pgm_string) }

#define MAKE_ENUM_MAP(mapping_name, enum_type, ...) constexpr auto mapping_name = make_mapping<enum_type>(__VA_ARGS__);

template<typename Enum, std::size_t N>
constexpr int get_buffer_size(const std::array<EnumMapping<Enum>, N> &mapping, int index) {
  return (index+1 == N)
         ? mapping[index].length
         : (mapping[index].length > get_buffer_size(mapping, index+1))
           ? mapping[index].length
           : get_buffer_size(mapping, index+1);
}

template<typename Enum, std::size_t N>
constexpr int get_buffer_size(const std::array<EnumMapping<Enum>, N> &mapping) {
  return get_buffer_size(mapping, 0);
}

// pgm str -> enum
template<typename Enum, std::size_t N>
inline Enum pgm_string_to_enum(const char *string_value,
                               Enum default_value,
                               const std::array<EnumMapping<Enum>, N> &mapping) {
  unsigned int buf_len = std::strlen(string_value) + 1;
  char buf[buf_len];
  for (int i = 0; i < (int) N; ++i) {
    if (mapping[i].length == buf_len && std::strcmp(string_value, read_prog_str(mapping[i].pgm_string, buf)) == 0) {
      return mapping[i].enum_value;
    }
  }
  return default_value;
}

// enum -> pgm str
template<typename Enum, std::size_t N>
inline String enum_to_pgm_string(Enum enum_value,
                                 const char *default_value,
                                 int default_value_length,
                                 const std::array<EnumMapping<Enum>, N> &mapping) {
  for (int i = 0; i < (int) N; ++i) {
    if (mapping[i].enum_value == enum_value) {
      char buf[mapping[i].length];
      return read_prog_str(mapping[i].pgm_string, buf);
    }
  }
  char buf[default_value_length];
  return read_prog_str(default_value, buf);
}

#endif //SPHUE_INCLUDE_ENUMTOOLS_HPP_
