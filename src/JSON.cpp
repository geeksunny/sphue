#include <utility>
#include <algorithm>

#include "JSON.h"

namespace json {

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
  value_ = false;
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