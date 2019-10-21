#include <utility>
#include <algorithm>

#include "JSON.h"

namespace json {

int strToInt(String &value) {
  bool negative = false;
  int result = 0;
  int i = 0;
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


bool JsonParser::parse(JsonModel &dest) {
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
  return findChar('"', ',') && getString(dest);
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


bool JsonParser::getBool(bool &dest) {
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


bool JsonParser::getInt(int &dest) {
  char next;
  String value;
  bool foundExponent = false;
  while (src_.available()) {
    next = src_.peek();
    switch (next) {
      case '-':
        if (value.length()) {
          // Sign was not the first character processed. Abort read.
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
      case 'e':
      case 'E':
        foundExponent = true;
      default:
        goto CHECK_VALUE;
    }
  }
  CHECK_VALUE:
  if (value.length()) {
    dest = strToInt(value);
    if (foundExponent) {
      int exponent;
      if (getExponent(exponent)) {
        // TODO: should long be used since std::pow returns double? is long a good alternative to double here?
        dest = (int) std::pow(dest, exponent);
      } else {
        // TODO: should anything happen if getExponent fails?
      }
      skipValue();
      return true;
    }
  }
  skipValue();
  return false;
}


bool JsonParser::getExponent(int &dest) {
  char next;
  String value;
  while (src_.available()) {
    next = src_.peek();
    switch (next) {
      case 'e':
      case 'E':
      case '+':
        src_.read();
        break;
      case '-':
        if (value.length()) {
          // Sign was not the first character processed. Abort read.
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


bool JsonParser::getString(String &dest) {
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
  }
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


////////////////////////////////////////////////////////////////
// Class : JsonObject //////////////////////////////////////////
////////////////////////////////////////////////////////////////

void JsonObject::add(String &key, JsonSerializable value) {
  values_[key] = std::move(value);
}


void JsonObject::add(String &key, String &value) {
  values_[key] = std::move(JsonString(value));
}


void JsonObject::add(String &key, bool value) {
  values_[key] = std::move(JsonBool(value));
}


void JsonObject::add(String &key, int value) {
  values_[key] = std::move(JsonNumber(value));
}


void JsonObject::add(String &key, double value) {
  values_[key] = std::move(JsonNumber(value));
}


void JsonObject::add(String &key, float value) {
  values_[key] = std::move(JsonNumber(value));
}


bool JsonObject::remove(String &key) {
  return values_.erase(key);
}


bool JsonObject::has(String &key) {
  return values_.count(key);
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
    result += (*it).second.toJson();
  }
  result += "}";
  return result;
}

}