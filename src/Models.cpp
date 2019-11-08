#include "Models.h"
#include <ctime>

#define CASE_RETURN(case_value, return_value)             case case_value: return return_value
#define IF_EQ_RET(compare_a, compare_b, return_value)     if (compare_a == compare_b) { return return_value; }

namespace sphue {

bool stringHasChar(String &string, char find) {
  for (char c : string) {
    if (c == find) {
      return true;
    }
  }
  return false;
}

bool timestampFromString(String &time, long &dest) {
  // `time` should be a datetime string formatted as YYYY-MM-DDThh:mm:ss
  if (time.length() < 19) {
    return false;
  }
  const char *str = time.c_str();
  char *end;
  std::tm t = {};
  t.tm_year = strtol(&str[0], &end, 10) - 1900;
  t.tm_mon = strtol(&str[5], &end, 10) - 1;
  t.tm_mday = strtol(&str[8], &end, 10);
  t.tm_hour = strtol(&str[11], &end, 10);
  t.tm_min = strtol(&str[14], &end, 10);
  t.tm_sec = strtol(&str[17], &end, 10);
  dest = mktime(&t);
  return true;
}

bool parseArrayOfIntStrings(json::JsonParser &parser, std::vector<int> &dest) {
  if (parser.checkValueType() != json::ARRAY) {
    parser.skipValue();
    return false;
  }
  String value;
  json::JsonArrayIterator<String> array = parser.iterateArray<String>();
  while (array.hasNext()) {
    if (array.getNext(value)) {
      dest.push_back(value.toInt());
    }
  }
  array.finish();
  return true;
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
// Class : ParsedMap ///////////////////////////////////////////
////////////////////////////////////////////////////////////////

template<typename K, typename T>
std::map<K, T> &ParsedMap<K, T>::operator*() {
  return values_;
}


template<typename K, typename T>
const std::map<K, T> &ParsedMap<K, T>::operator*() const {
  return values_;
}


template<typename K, typename T>
bool ParsedMap<K, T>::onKey(String &key, json::JsonParser &parser) {
  int id = key.toInt();
  T value;
  bool success = parser.get(value);
  values_[id] = value;
  return success;
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
// Class : NewLights ///////////////////////////////////////////
////////////////////////////////////////////////////////////////

long NewLights::lastscan() const {
  return lastscan_;
}


bool NewLights::isScanning() const {
  return lastscan_ == -1;
}


bool NewLights::onKey(String &key, json::JsonParser &parser) {
  bool success;
  if (key == "lastscan") {
    String value;
    success = parser.get(value);
    if (success) {
      if (value == "none") {
        lastscan_ = 0;
      } else if (value == "active") {
        lastscan_ = -1;
      } else {
        success = timestampFromString(value, lastscan_);
      }
    }
  } else if (key == "name") {
    String name;
    success = parser.get(name);
    if (success) {
      values_[last_parsed_id_] = name;
      last_parsed_id_ = 0;
    }
  } else {
    last_parsed_id_ = key.toInt();
    success = parser.get(*this);
  }
  return success;
}


////////////////////////////////////////////////////////////////
// Class : LightStateChange ////////////////////////////////////
////////////////////////////////////////////////////////////////

void LightStateChange::setOn(bool turned_on) {
  String key = "on";
  add(key, turned_on);
}


void LightStateChange::setBrightness(uint8_t brightness) {
  String key = "bri";
  add(key, brightness);
}


void LightStateChange::setHue(uint16_t hue) {
  String key = "hue";
  add(key, hue);
}


void LightStateChange::setSaturation(uint8_t saturation) {
  String key = "sat";
  add(key, saturation);
}


void LightStateChange::setColorTemp(uint16_t color_temp) {
  String key = "ct";
  add(key, color_temp);
}


void LightStateChange::setTransitionTime(uint16_t time_in_tenths_of_seconds) {
  String key = "transitiontime";
  add(key, time_in_tenths_of_seconds);
}


void LightStateChange::incrementBrightness(uint8_t brightness_increment) {
  String key = "bri_inc";
  add(key, brightness_increment);
}


void LightStateChange::decrementBrightness(uint8_t brightness_decrement) {
  String key = "bri_inc";
  add(key, -brightness_decrement);
}


void LightStateChange::incrementSaturation(uint8_t saturation_increment) {
  String key = "sat_inc";
  add(key, saturation_increment);
}


void LightStateChange::decrementSaturation(uint8_t saturation_decrement) {
  String key = "sat_inc";
  add(key, -saturation_decrement);
}


void LightStateChange::incrementHue(uint16_t hue_increment) {
  String key = "hue_inc";
  add(key, hue_increment);
}


void LightStateChange::decrementHue(uint16_t hue_decrement) {
  String key = "hue_inc";
  add(key, -hue_decrement);
}


void LightStateChange::incrementColorTemp(uint16_t color_temp_increment) {
  String key = "ct_inc";
  add(key, color_temp_increment);
}


void LightStateChange::decrementColorTemp(uint16_t color_temp_decrement) {
  String key = "ct_inc";
  add(key, -color_temp_decrement);
}


////////////////////////////////////////////////////////////////
// Class : Group ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

Group::Type Group::typeFromString(String &string) {
  IF_EQ_RET(string, "Luminaire", Type::LUMINAIRE)
  IF_EQ_RET(string, "Lightsource", Type::LIGHTSOURCE)
  IF_EQ_RET(string, "LightGroup", Type::LIGHT_GROUP)
  IF_EQ_RET(string, "Room", Type::ROOM)
  IF_EQ_RET(string, "Entertainment", Type::ENTERTAINMENT)
  IF_EQ_RET(string, "Zone", Type::ZONE)
  // else
  return Type::UNKNOWN;
}

String Group::typeToString(Group::Type &type) {
  switch (type) {
    CASE_RETURN(Type::LUMINAIRE, "Luminaire");
    CASE_RETURN(Type::LIGHTSOURCE, "Lightsource");
    CASE_RETURN(Type::LIGHT_GROUP, "LightGroup");
    CASE_RETURN(Type::ROOM, "Room");
    CASE_RETURN(Type::ENTERTAINMENT, "Entertainment");
    CASE_RETURN(Type::ZONE, "Zone");
    default:
      return "Unknown";
  }
}

Group::Class Group::classFromString(String &string) {
  IF_EQ_RET(string, "Living room", Class::LIVING_ROOM)
  IF_EQ_RET(string, "Kitchen", Class::KITCHEN)
  IF_EQ_RET(string, "Dining", Class::DINING)
  IF_EQ_RET(string, "Bedroom", Class::BEDROOM)
  IF_EQ_RET(string, "Kids bedroom", Class::KIDS_BEDROOM)
  IF_EQ_RET(string, "Bathroom", Class::BATHROOM)
  IF_EQ_RET(string, "Nursery", Class::NURSERY)
  IF_EQ_RET(string, "Recreation", Class::RECREATION)
  IF_EQ_RET(string, "Office", Class::OFFICE)
  IF_EQ_RET(string, "Gym", Class::GYM)
  IF_EQ_RET(string, "Hallway", Class::HALLWAY)
  IF_EQ_RET(string, "Toilet", Class::TOILET)
  IF_EQ_RET(string, "Front door", Class::FRONT_DOOR)
  IF_EQ_RET(string, "Garage", Class::GARAGE)
  IF_EQ_RET(string, "Terrace", Class::TERRACE)
  IF_EQ_RET(string, "Garden", Class::GARDEN)
  IF_EQ_RET(string, "Driveway", Class::DRIVEWAY)
  IF_EQ_RET(string, "Carport", Class::CARPORT)
  IF_EQ_RET(string, "Other", Class::OTHER)
  IF_EQ_RET(string, "Home", Class::HOME)
  IF_EQ_RET(string, "Downstairs", Class::DOWNSTAIRS)
  IF_EQ_RET(string, "Upstairs", Class::UPSTAIRS)
  IF_EQ_RET(string, "Top floor", Class::TOP_FLOOR)
  IF_EQ_RET(string, "Attic", Class::ATTIC)
  IF_EQ_RET(string, "Guest room", Class::GUEST_ROOM)
  IF_EQ_RET(string, "Staircase", Class::STAIRCASE)
  IF_EQ_RET(string, "Lounge", Class::LOUNGE)
  IF_EQ_RET(string, "Man cave", Class::MAN_CAVE)
  IF_EQ_RET(string, "Computer", Class::COMPUTER)
  IF_EQ_RET(string, "Studio", Class::STUDIO)
  IF_EQ_RET(string, "Music", Class::MUSIC)
  IF_EQ_RET(string, "TV", Class::TV)
  IF_EQ_RET(string, "Reading", Class::READING)
  IF_EQ_RET(string, "Closet", Class::CLOSET)
  IF_EQ_RET(string, "Storage", Class::STORAGE)
  IF_EQ_RET(string, "Laundry room", Class::LAUNDRY_ROOM)
  IF_EQ_RET(string, "Balcony", Class::BALCONY)
  IF_EQ_RET(string, "Porch", Class::PORCH)
  IF_EQ_RET(string, "Barbecue", Class::BARBECUE)
  IF_EQ_RET(string, "Pool", Class::POOL)
  // else
  return Class::UNKNOWN;
}

String Group::classToString(Group::Class &a_class) {
  switch (a_class) {
    CASE_RETURN(Class::LIVING_ROOM, "Living room");
    CASE_RETURN(Class::KITCHEN, "Kitchen");
    CASE_RETURN(Class::DINING, "Dining");
    CASE_RETURN(Class::BEDROOM, "Bedroom");
    CASE_RETURN(Class::KIDS_BEDROOM, "Kids bedroom");
    CASE_RETURN(Class::BATHROOM, "Bathroom");
    CASE_RETURN(Class::NURSERY, "Nursery");
    CASE_RETURN(Class::RECREATION, "Recreation");
    CASE_RETURN(Class::OFFICE, "Office");
    CASE_RETURN(Class::GYM, "Gym");
    CASE_RETURN(Class::HALLWAY, "Hallway");
    CASE_RETURN(Class::TOILET, "Toilet");
    CASE_RETURN(Class::FRONT_DOOR, "Front door");
    CASE_RETURN(Class::GARAGE, "Garage");
    CASE_RETURN(Class::TERRACE, "Terrace");
    CASE_RETURN(Class::GARDEN, "Garden");
    CASE_RETURN(Class::DRIVEWAY, "Driveway");
    CASE_RETURN(Class::CARPORT, "Carport");
    CASE_RETURN(Class::OTHER, "Other");
    CASE_RETURN(Class::HOME, "Home");
    CASE_RETURN(Class::DOWNSTAIRS, "Downstairs");
    CASE_RETURN(Class::UPSTAIRS, "Upstairs");
    CASE_RETURN(Class::TOP_FLOOR, "Top floor");
    CASE_RETURN(Class::ATTIC, "Attic");
    CASE_RETURN(Class::GUEST_ROOM, "Guest room");
    CASE_RETURN(Class::STAIRCASE, "Staircase");
    CASE_RETURN(Class::LOUNGE, "Lounge");
    CASE_RETURN(Class::MAN_CAVE, "Man cave");
    CASE_RETURN(Class::COMPUTER, "Computer");
    CASE_RETURN(Class::STUDIO, "Studio");
    CASE_RETURN(Class::MUSIC, "Music");
    CASE_RETURN(Class::TV, "TV");
    CASE_RETURN(Class::READING, "Reading");
    CASE_RETURN(Class::CLOSET, "Closet");
    CASE_RETURN(Class::STORAGE, "Storage");
    CASE_RETURN(Class::LAUNDRY_ROOM, "Laundry room");
    CASE_RETURN(Class::BALCONY, "Balcony");
    CASE_RETURN(Class::PORCH, "Porch");
    CASE_RETURN(Class::BARBECUE, "Barbecue");
    CASE_RETURN(Class::POOL, "Pool");
    default:
      return "Unknown";
  }
}


const String &Group::name() const {
  return name_;
}


const std::vector<int> &Group::lights() const {
  return lights_;
}


const std::vector<int> &Group::sensors() const {
  return sensors_;
}


bool Group::allOn() const {
  return all_on_;
}


bool Group::anyOn() const {
  return any_on_;
}


bool Group::recycle() const {
  return recycle_;
}


const State &Group::action() const {
  return action_;
}


bool Group::onKey(String &key, json::JsonParser &parser) {
  if (key == "name") {
    return parser.get(name_);
  } else if (key == "lights") {
    return parseArrayOfIntStrings(parser, lights_);
  } else if (key == "sensors") {
    return parseArrayOfIntStrings(parser, sensors_);
  } else if (key == "type") {
    String type;
    bool success = parser.get(type);
    type_ = typeFromString(type);
    return success;
  } else if (key == "state") {
    return parser.get(*this);
  } else if (key == "all_on") {
    return parser.get(all_on_);
  } else if (key == "any_on") {
    return parser.get(any_on_);
  } else if (key == "recycle") {
    return parser.get(recycle_);
  } else if (key == "class") {
    String a_class;
    bool success = parser.get(a_class);
    class_ = classFromString(a_class);
    return success;
  } else if (key == "action") {
    return parser.get(action_);
  }
  return false;
}

}