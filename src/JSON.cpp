#include <Arduino.h>
#include <algorithm>

#include "JSON.h"

namespace json {

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args ) {
  return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}


unsigned long strToLong(String &value) {
  bool negative = false;
  int result = 0;
  unsigned int i = 0;
  if (value[0] == '-') {
    negative = true;
    i = 1;
  }
  char c;
  for (; i < value.length(); ++i) {
    c = value[i];
    if (!isdigit(c)) {
      break;
    }
    result = (result * 10) + (c - '0');
  }
  return (negative) ? -result : result;
}


////////////////////////////////////////////////////////////////
// Class : JsonParser //////////////////////////////////////////
////////////////////////////////////////////////////////////////

JsonParser::JsonParser(Stream &src) : src_(src) {
  //
}


bool JsonParser::get(JsonModel &dest) {
  yield();
  if (findObject()) {
    src_.read();
    String key;
    while (findNextKey(key)) {
      if (!findValue()) {
        // Cannot find a value after this key!
        // TODO: How should we handle this situation? skip to end of object and return false?
      }
      if (!dest.onKey(key, *this)) {
        // If dest.onKey fails, skip to the next value.
        skipValue();
      }
      key.clear();
      yield();
    }
    if (findChar('}')) {
      src_.read();
      return true;
    }
  }
  return false;
}


bool JsonParser::findChar(const unsigned char find, const bool skipWhitespace) {
  unsigned char next;
  while (src_.available()) {
    next = src_.peek();
    if (skipWhitespace && isspace(next)) {
      // Skipping occurrences of whitespace characters
      src_.read();
    } else {
      return next == find;
    }
  }
  return false;
}


bool JsonParser::findChar(const unsigned char find, const char skipChar, const bool skipWhitespace) {
  unsigned char next;
  while (src_.available()) {
    next = src_.peek();
    if (next == skipChar || (skipWhitespace && isspace(next))) {
      // Skipping occurrences of `skipChar` and whitespace characters
      src_.read();
    } else {
      return next == find;
    }
  }
  return false;
}


bool JsonParser::findChar(const unsigned char find, const char *skipChars, const bool skipWhitespace) {
  unsigned char next;
  while (src_.available()) {
    next = src_.peek();
    if (strchr(skipChars, next) != nullptr || (skipWhitespace && isspace(next))) {
      // Skipping occurrences of characters in `skipChars` and whitespace characters
      src_.read();
    } else {
      return next == find;
    }
  }
  return false;
}


JsonValueType JsonParser::checkValueType() {
  return checkValueType(src_.peek());
}


JsonValueType JsonParser::checkValueType(unsigned char firstChar) {
  switch (firstChar) {
    case '"':
      return STRING;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return NUMBER;
    case '{':
      return OBJECT;
    case '[':
      return ARRAY;
    case 't':
    case 'f':
      return BOOL;
    case 'n':
      return NUL;
    default:
      return INVALID;
  }
}


bool JsonParser::findArray() {
  return findChar('[');
}


bool JsonParser::findObject() {
  return findChar('{');
}


bool JsonParser::findNextKey(String &dest) {
  return findChar('"', ',') && get(dest);
}


bool JsonParser::peekMatches(unsigned char c) {
  return src_.available() && src_.peek() == c;
}


bool JsonParser::readMatches(unsigned char c) {
  if (src_.available() && src_.peek() == c) {
    src_.read();
    return true;
  }
  return false;
}


bool JsonParser::readMatches(const char *value, bool case_sensitive) {
  unsigned char srcNext, valueNext;
  for (int i = 0; value[i] != '\0'; ++i) {
    if (case_sensitive) {
      srcNext = src_.peek();
      valueNext = value[i];
    } else {
      srcNext = tolower(src_.peek());
      valueNext = tolower(value[i]);
    }
    if (srcNext != valueNext) {
      // Next characters don't match; error result.
      return false;
    }
    src_.read();
  }
  return true;
}


bool JsonParser::skipValue() {
  char skipTo;
  while (src_.available()) {
    switch (src_.peek()) {
      case '{':
        skipTo = '}';
        break;
      case '[':
        skipTo = ']';
        break;
      case ',':
      case ']':
      case '}':
        return true;
      default:
        src_.read();
        continue;
    }
    src_.read();
    if (skipToChar(skipTo, true)) {
      src_.read();
    } else {
      break;
    }
  }
  return false;
}


bool JsonParser::skipToChar(unsigned char skipTo, bool recursive) {
  yield();
  unsigned char c;
  while (src_.available()) {
    c = src_.peek();
    if (c == skipTo) {
      return true;
    } else if (recursive) {
      // If c matches a bracket, read from src_ before performing a recursive skip operation.
      // TODO: Should this logic be broken out to improve readability?
      if ((c == '{' && (src_.read(), !skipToChar('}', true))) ||
          (c == '[' && (src_.read(), !skipToChar(']', true)))) {
        break;
      }
    }
    src_.read();
  }
  return false;
}


bool JsonParser::findValue() {
  unsigned char next;
  while (src_.available()) {
    next = src_.peek();
    if (next == ':' || isspace(next)) {
      // Skipping occurrences of ':' and whitespace characters
      src_.read();
    } else {
      return true;
    }
  }
  return false;
}


bool JsonParser::get(bool &dest) {
  if (src_.available()) {
    switch (src_.peek()) {
      case 't':
        if (readMatches("true")) {
          dest = true;
          return true;
        }
        break;
      case 'f':
        if (readMatches("false")) {
          dest = false;
          return true;
        }
    }
    skipValue();
  }
  return false;
}


bool JsonParser::get(unsigned long &dest) {
  // Parse the number as into a double, cast result to int on success.
  double value;
  if (get(value)) {
    dest = (unsigned long) value;
    return true;
  }
  return false;
}


bool JsonParser::get(double &dest) {
  unsigned long value;
  if (getDigits(value, true)) {
    // Whole number
    dest = (double) value;
    unsigned char next = src_.peek();
    // Precision
    if (next == '.') {
      double precision;
      if (getPrecision(precision)) {
        dest += precision;
      }
      next = src_.peek();
    }
    // Exponent
    if (next == 'e' || next == 'E') {
      double exponent;
      if (getExponent(exponent)) {
        dest = std::pow(dest, exponent);
      }
    }
    // Finished parsing
    skipValue();
    return true;
  }
  skipValue();
  return false;
}


bool JsonParser::getDigits(unsigned long &dest, const bool allow_sign) {
  String value;
  while (src_.available()) {
    switch (src_.peek()) {
      case '-':
        if (!allow_sign || value.length()) {
          // Sign not allowed or is not the first character processed. Abort read.
          goto CHECK_VALUE;
        }
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        value.concat((char) src_.read());
        yield();
        break;
      default:
        goto CHECK_VALUE;
    }
  }
  CHECK_VALUE:
  if (value.length()) {
    dest = strToLong(value);
    return true;
  }
  return false;
}


bool JsonParser::getExponent(double &dest) {
  if (src_.available()) {
    // Skip exponent symbol
    unsigned char next = src_.peek();
    if (next == 'e' || next == 'E') {
      src_.read();
      next = src_.peek();
    }
    // Skip positive sign; It is legal in the exponent and can be ignored.
    if (next == '+') {
      src_.read();
    }
    // Whole number
    unsigned long value;
    if (getDigits(value, true)) {
      dest = (double) value;
      // Precision
      if (src_.peek() == '.') {
        double precision;
        if (getPrecision(precision)) {
          dest += precision;
        }
      }
      return true;
    }
  }
  return false;
}


bool JsonParser::getPrecision(double &dest) {
  if (src_.available()) {
    if (src_.peek() == '.') {
      src_.read();
    }
    unsigned long value;
    if (getDigits(value, false)) {
      dest = (double) value;
      while (dest >= 1.0) {
        dest /= 10;
      }
      return true;
    }
  }
  return false;
}


bool JsonParser::get(String &dest) {
  if (!src_.available() || src_.peek() != '"') {
    return false;
  }
  src_.read();
  unsigned char c;
  bool ignoreNext = false;
  while (src_.available()) {
    c = src_.read();
    if (c == '\\' && !ignoreNext) {
      ignoreNext = true;
      dest.concat((char) c);
      yield();
      continue;
    }
    if (c == '"' && !ignoreNext) {
      return true;
    } else {
      dest.concat((char) c);
      yield();
      ignoreNext = false;
    }
  }
  return false;
}


bool JsonParser::getAsString(String &dest) {
  switch (checkValueType()) {
    case STRING:
      return get(dest);
    case NUMBER:
    case BOOL:
    case NUL: {
      unsigned char c;
      while (src_.available()) {
        c = src_.peek();
        if (c == ',' || c == '}' || c == ']' || isspace(c)) {
          // Reached break / end of value. Success on one-or-more characters read into destination String.
          return dest.length();
        } else {
          dest.concat((char) c);
          yield();
        }
      }
      return false;
    }
    case OBJECT:
    case ARRAY:
    case INVALID:
    default:
      // TODO: Not implemented!
      return false;
  }
}

////////////////////////////////////////////////////////////////
// Class : JsonString //////////////////////////////////////////
////////////////////////////////////////////////////////////////

JsonString::JsonString(String &value) : value_(value) {
  //
}


const String &JsonString::getValue() const {
  return value_;
}


void JsonString::setValue(String &value) {
  value_ = value;
}


String JsonString::toJson() {
  String result{};
  result.reserve(value_.length() + 2);
  result.concat('\"');
  result.concat(value_);
  result.concat('\"');
  return result;
}


bool JsonString::operator==(const JsonString &rhs) const {
  return value_ == rhs.value_;
}


bool JsonString::operator!=(const JsonString &rhs) const {
  return value_ != rhs.value_;
}


////////////////////////////////////////////////////////////////
// Class : JsonBool ////////////////////////////////////////////
////////////////////////////////////////////////////////////////

JsonBool::JsonBool(bool value) : value_(value) {
  //
}


bool JsonBool::getValue() {
  return value_;
}


void JsonBool::setValue(bool value) {
  value_ = value;
}


String JsonBool::toJson() {
  return value_ ? "true" : "false";
}


bool JsonBool::operator==(const JsonBool &rhs) const {
  return value_ == rhs.value_;
}


bool JsonBool::operator!=(const JsonBool &rhs) const {
  return value_ != rhs.value_;
}


////////////////////////////////////////////////////////////////
// Class : JsonNumber //////////////////////////////////////////
////////////////////////////////////////////////////////////////

JsonNumber::JsonNumber(int value) {
  int_value_ = value;
  type_ = INT;
  double_value_ = 0;
  float_value_ = 0;
}


JsonNumber::JsonNumber(double value) {
  double_value_ = value;
  type_ = DOUBLE;
  int_value_ = 0;
  float_value_ = 0;
}


JsonNumber::JsonNumber(float value) {
  float_value_ = value;
  type_ = FLOAT;
  int_value_ = 0;
  double_value_ = 0;
}


int JsonNumber::getInt() {
  switch (type_) {
    case INT:
      return int_value_;
    case DOUBLE:
      return (int) double_value_;
    case FLOAT:
      return (int) float_value_;
    default:
      return 0;
  }
}


double JsonNumber::getDouble() {
  switch (type_) {
    case INT:
      return int_value_;
    case DOUBLE:
      return double_value_;
    case FLOAT:
      return float_value_;
    default:
      return 0;
  }
}


float JsonNumber::getFloat() {
  switch (type_) {
    case INT:
      return int_value_;
    case DOUBLE:
      return double_value_;
    case FLOAT:
      return float_value_;
    default:
      return 0;
  }
}


void JsonNumber::setValue(int value) {
  int_value_ = value;
  type_ = INT;
}


void JsonNumber::setValue(double value) {
  double_value_ = value;
  type_ = DOUBLE;
}


void JsonNumber::setValue(float value) {
  float_value_ = value;
  type_ = FLOAT;
}


void JsonNumber::setType(JsonNumberType type) {
  if (type_ == type) {
    return;
  }
  switch (type) {
    case INT: {
      if (type_ == DOUBLE) {
        int_value_ = (int) double_value_;
      } else if (type_ == FLOAT) {
        int_value_ = (int) float_value_;
      }
      break;
    }
    case DOUBLE: {
      if (type_ == INT) {
        double_value_ = int_value_;
      } else if (type_ == FLOAT) {
        double_value_ = float_value_;
      }
      break;
    }
    case FLOAT: {
      if (type_ == INT) {
        float_value_ = int_value_;
      } else if (type_ == DOUBLE) {
        float_value_ = double_value_;
      }
      break;
    }
  }
}


String JsonNumber::toJson() {
  switch (type_) {
    case INT:
      return String(int_value_);
    case DOUBLE:
      // TODO: add formatting rules for precision?
      return String(double_value_);
    case FLOAT:
      // TODO: add formatting rules for precision?
      return String(float_value_);
    default:
      return "";
  }
}


bool JsonNumber::operator==(const JsonNumber &rhs) const {
  if (type_ == rhs.type_) {
    switch (rhs.type_) {
      case INT:
        return int_value_ == rhs.int_value_;
      case DOUBLE:
        return double_value_ == rhs.double_value_;
      case FLOAT:
        return float_value_ == rhs.float_value_;
    }
  }
  return false;
}


bool JsonNumber::operator!=(const JsonNumber &rhs) const {
  return !(rhs == *this);
}


////////////////////////////////////////////////////////////////
// Class : JsonArray ///////////////////////////////////////////
////////////////////////////////////////////////////////////////

template<typename SerializableType>
JsonArray<SerializableType>::JsonArray(int initial_capacity) : values_(initial_capacity) {
  //
}


template<typename SerializableType>
void JsonArray<SerializableType>::add(SerializableType &value) {
  values_.push_back(value);
}


template<typename SerializableType>
bool JsonArray<SerializableType>::remove(SerializableType &value) {
  auto entry = std::find(values_.begin(), values_.end(), value);
  if (entry == values_.end()) {
    return false;
  }
  values_.erase(entry);
  return true;
}


template<typename SerializableType>
String JsonArray<SerializableType>::toJson() {
  String result = "[";
  for (auto it = values_.begin(); it != values_.end(); ++it) {
    String value = (*it)->toJson();
    // Reserve additional space on result string; Prevent extra reserve operations from happening on each concatenation.
    unsigned int concat_len = (*it).first.length() + value.length() + 1;
    if (it != values_.begin()) {
      result.reserve(result.length() + concat_len + 1);
      result += ',';
    } else {
      result.reserve(result.length() + concat_len);
    }
    result += value;
  }
  result += ']';
  return result;
}


template<typename SerializableType>
bool JsonArray<SerializableType>::operator==(const JsonArray &rhs) const {
  return values_ == rhs.values_;
}


template<typename SerializableType>
bool JsonArray<SerializableType>::operator!=(const JsonArray &rhs) const {
  return !(rhs == *this);
}


////////////////////////////////////////////////////////////////
// Class : JsonObject //////////////////////////////////////////
////////////////////////////////////////////////////////////////

template<typename T>
void JsonObject::add(String &key, T &value) {
  values_[key] = make_unique<T>(value);
}


void JsonObject::add(String &key, String &value) {
  values_[key] = make_unique<JsonString>(value);
}


void JsonObject::add(String &key, bool value) {
  values_[key] = make_unique<JsonBool>(value);
}


void JsonObject::add(String &key, int value) {
  values_[key] = make_unique<JsonNumber>(value);
}


void JsonObject::add(String &key, double value) {
  values_[key] = make_unique<JsonNumber>(value);
}


void JsonObject::add(String &key, float value) {
  values_[key] = make_unique<JsonNumber>(value);
}


bool JsonObject::remove(String &key) {
  return values_.erase(key);
}


bool JsonObject::has(String &key) {
  return values_.count(key);
}


int JsonObject::size() {
  return values_.size();
}


String JsonObject::toJson() {
  String result = "{";
  for (auto it = values_.begin(); it != values_.end(); ++it) {
    String value = (*it).second->toJson();
    // Reserve additional space on result string; Prevent extra reserve operations from happening on each concatenation.
    unsigned int concat_len = (*it).first.length() + value.length() + 4;
    if (it != values_.begin()) {
      result.reserve(result.length() + concat_len + 1);
      result += ',';
    } else {
      result.reserve(result.length() + concat_len);
    }
    result += '\"';
    result += (*it).first;
    result += "\":";
    result += value;
  }
  result += '}';
  return result;
}


bool JsonObject::operator==(const JsonObject &rhs) const {
  return values_ == rhs.values_;
}


bool JsonObject::operator!=(const JsonObject &rhs) const {
  return !(rhs == *this);
}

}