#include <algorithm>

#include "JSON.h"

namespace json {

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args ) {
  return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}


int strToInt(String &value) {
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
  if (findObject()) {
    String key;
    while (findNextKey(key)) {
      if (!findValue()) {
        // Cannot find a value after this key!
        // TODO: How should we handle this situation? skip to end of object and return false?
      }
      dest.onKey(key, *this);
    }
    if (findChar('}')) {
      return true;
    }
  }
  return false;
}


bool JsonParser::findChar(const char find, const bool skipWhitespace) {
  char next;
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


bool JsonParser::findChar(const char find, const char skipChar, const bool skipWhitespace) {
  char next;
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


bool JsonParser::findChar(const char find, const char *skipChars, const bool skipWhitespace) {
  char next;
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


JsonValueType JsonParser::checkValueType(char firstChar) {
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


bool JsonParser::findValue() {
  char next;
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


bool JsonParser::peekMatches(char c) {
  return src_.available() && src_.peek() == c;
}


bool JsonParser::readMatches(char c) {
  if (src_.available() && src_.peek() == c) {
    src_.read();
    return true;
  }
  return false;
}


bool JsonParser::readMatches(const char *value, bool case_sensitive) {
  char srcNext, valueNext;
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
  while (src_.available()) {
    switch (src_.peek()) {
      case ',':
      case ']':
      case '}':
        return true;
      default:
        src_.read();
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


bool JsonParser::get(int &dest) {
  // Parse the number as into a double, cast result to int on success.
  double value;
  if (get(value)) {
    dest = (int) value;
    return true;
  }
  return false;
}


bool JsonParser::get(double &dest) {
  int value;
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


bool JsonParser::getDigits(int &dest, const bool allow_sign) {
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
        value.concat(src_.read());
        break;
      default:
        goto CHECK_VALUE;
    }
  }
  CHECK_VALUE:
  if (value.length()) {
    dest = strToInt(value);
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
    int value;
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
    int value;
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
  char c;
  bool ignoreNext = false;
  while (src_.available()) {
    c = src_.read();
    if (c == '\\' && !ignoreNext) {
      ignoreNext = true;
      dest.concat(c);
      continue;
    }
    if (c == '"' && !ignoreNext) {
      return true;
    } else {
      dest.concat(c);
      ignoreNext = false;
    }
  }
  return false;
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
  return "\"" + value_ + "\"";
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
    if (it != values_.begin()) {
      result += ",";
    }
    result += (*it)->toJson();
  }
  result += "]";
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
    if (it != values_.begin()) {
      result += ",";
    }
    result += "\"";
    result += (*it).first;
    result += "\":";
    result += (*it).second->toJson();
  }
  result += "}";
  return result;
}


bool JsonObject::operator==(const JsonObject &rhs) const {
  return values_ == rhs.values_;
}


bool JsonObject::operator!=(const JsonObject &rhs) const {
  return !(rhs == *this);
}

}