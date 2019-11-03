#include "Models.h"

namespace sphue {

bool stringHasChar(String &string, char find) {
  for (char c : string) {
    if (c == find) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////
// Class : NamedValue //////////////////////////////////////////
////////////////////////////////////////////////////////////////

NamedValue::NamedValue(std::shared_ptr<String> &prefix) : prefix_(prefix), type_(NamedValue::Type::UNKNOWN) {
  //
}


bool NamedValue::onKey(String &key, json::JsonParser &parser) {
  if (prefix_) {
    int index = key.indexOf(*prefix_.get());
    if (index == -1) {
      name_ = key;
    } else {
      name_ = key.substring(index + prefix_.get()->length());
    }
  } else {
    name_ = key;
  }
  bool success;
  switch (parser.checkValueType()) {
    case json::BOOL:
      type_ = Type::BOOL;
      bool value;
      success = parser.get(value);
      value_ = value ? "1" : "0";
      break;
    case json::NUMBER:
      success = parser.getAsString(value_);
      type_ = stringHasChar(value_, '.') ? Type::FLOAT : Type::INT;
      break;
    case json::STRING:
      type_ = Type::STRING;
      success = parser.get(value_);
      break;
    case json::INVALID:
    case json::OBJECT:
    case json::ARRAY:
    case json::NUL:
    default:
      success = false;
      break;
  }
  return success;
}


const String &NamedValue::getPrefix() const {
  return *prefix_.get();
}


NamedValue::Type NamedValue::getType() const {
  return type_;
}


const String &NamedValue::getName() const {
  return name_;
}


const String NamedValue::getFullName() const {
  if (prefix_) {
    return *prefix_.get() + name_;
  } else {
    return name_;
  }
}


const String &NamedValue::getString() const {
  return value_;
}


const int NamedValue::getInt() const {
  return value_.toInt();
}


const float NamedValue::getFloat() const {
  return value_.toFloat();
}


const bool NamedValue::getBool() const {
  return value_ == "1";
}


////////////////////////////////////////////////////////////////
// Class : DiscoveryResponse ///////////////////////////////////
////////////////////////////////////////////////////////////////

bool DiscoveryResponse::onKey(String &key, json::JsonParser &parser) {
  if (key == "id") {
    return parser.get(id_);
  } else if (key == "internalipaddress") {
    return parser.get(ip_);
  }
  return false;
}


const String &DiscoveryResponse::id() const {
  return id_;
}


const String &DiscoveryResponse::ip() const {
  return ip_;
}


////////////////////////////////////////////////////////////////
// Class : RegisterResponse ////////////////////////////////////
////////////////////////////////////////////////////////////////

const String &RegisterResponse::username() const {
  return username_;
}


bool RegisterResponse::onKey(String &key, json::JsonParser &parser) {
  return (key == "username") ? parser.get(username_) : false;
}


////////////////////////////////////////////////////////////////
// Class : State ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

const State &Light::state() const {
  return state_;
}


const String &Light::name() const {
  return name_;
}


const String &Light::uniqueid() const {
  return uniqueid_;
}


bool State::onKey(String &key, json::JsonParser &parser) {
  if (key == "on") {
    return parser.get(on_);
  } else if (key == "bri") {
    return parser.get(bri_);
  } else if (key == "hue") {
    return parser.get(hue_);
  } else if (key == "sat") {
    return parser.get(sat_);
  } else if (key == "ct") {
    return parser.get(ct_);
  } else if (key == "reachable") {
    return parser.get(reachable_);
  }
  return false;
}


////////////////////////////////////////////////////////////////
// Class : Light ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

bool Light::onKey(String &key, json::JsonParser &parser) {
  if (key == "state") {
    return parser.get(state_);
  } else if (key == "name") {
    return parser.get(name_);
  } else if (key == "uniqueid") {
    return parser.get(uniqueid_);
  }
  return false;
}


////////////////////////////////////////////////////////////////
// Class : Lights //////////////////////////////////////////////
////////////////////////////////////////////////////////////////

std::map<int, Light> &Lights::operator*() {
  return lights_;
}


const std::map<int, Light> &Lights::operator*() const {
  return lights_;
}


bool Lights::onKey(String &key, json::JsonParser &parser) {
  int id = json::strToInt(key);
  Light light;
  bool success = parser.get(light);
  lights_[id] = light;
  return success;
}


////////////////////////////////////////////////////////////////
// Class : LightStateChange ////////////////////////////////////
////////////////////////////////////////////////////////////////

void LightStateChange:: setOn(bool turned_on) {
  String key = "on";
  add(key, turned_on);
}


void LightStateChange:: setBrightness(uint8_t brightness) {
  String key = "bri";
  add(key, brightness);
}


void LightStateChange:: setHue(uint16_t hue) {
  String key = "hue";
  add(key, hue);
}


void LightStateChange:: setSaturation(uint8_t saturation) {
  String key = "sat";
  add(key, saturation);
}


void LightStateChange:: setColorTemp(uint16_t color_temp) {
  String key = "ct";
  add(key, color_temp);
}


void LightStateChange:: setTransitionTime(uint16_t time_in_tenths_of_seconds) {
  String key = "transitiontime";
  add(key, time_in_tenths_of_seconds);
}


void LightStateChange:: incrementBrightness(uint8_t brightness_increment) {
  String key = "bri_inc";
  add(key, brightness_increment);
}


void LightStateChange:: decrementBrightness(uint8_t brightness_decrement) {
  String key = "bri_inc";
  add(key, -brightness_decrement);
}


void LightStateChange:: incrementSaturation(uint8_t saturation_increment) {
  String key = "sat_inc";
  add(key, saturation_increment);
}


void LightStateChange:: decrementSaturation(uint8_t saturation_decrement) {
  String key = "sat_inc";
  add(key, -saturation_decrement);
}


void LightStateChange:: incrementHue(uint16_t hue_increment) {
  String key = "hue_inc";
  add(key, hue_increment);
}


void LightStateChange:: decrementHue(uint16_t hue_decrement) {
  String key = "hue_inc";
  add(key, -hue_decrement);
}


void LightStateChange:: incrementColorTemp(uint16_t color_temp_increment) {
  String key = "ct_inc";
  add(key, color_temp_increment);
}


void LightStateChange:: decrementColorTemp(uint16_t color_temp_decrement) {
  String key = "ct_inc";
  add(key, -color_temp_decrement);
}

}