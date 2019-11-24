#include "Models.h"
#include <ctime>
#include "prog_str.h"

#define CASE_RETURN(case_value, return_progmem_str)     case case_value: return read_prog_str(return_progmem_str)
#define IF_EQ_RET(str, progmem_str, return_value)       if (str == read_prog_str(progmem_str)) { return return_value; }

namespace sphue {

namespace strings {
// JSON keys
const char key_all_on[] PROGMEM = "all_on";
const char key_any_on[] PROGMEM = "any_on";
const char key_action[] PROGMEM = "action";
const char key_class[] PROGMEM = "class";
const char key_bri[] PROGMEM = "bri";
const char key_bri_inc[] PROGMEM = "bri_inc";
const char key_bri_dec[] PROGMEM = "bri_dec";
const char key_ct[] PROGMEM = "ct";
const char key_ct_inc[] PROGMEM = "ct_inc";
const char key_ct_dec[] PROGMEM = "ct_dec";
const char key_group[] PROGMEM = "group";
const char key_hue[] PROGMEM = "hue";
const char key_hue_inc[] PROGMEM = "hue_inc";
const char key_hue_dec[] PROGMEM = "hue_dec";
const char key_id[] PROGMEM = "id";
const char key_internalipaddress[] PROGMEM = "internalipaddress";
const char key_lastscan[] PROGMEM = "lastscan";
const char key_lights[] PROGMEM = "lights";
const char key_locked[] PROGMEM = "locked";
const char key_name[] PROGMEM = "name";
const char key_on[] PROGMEM = "on";
const char key_reachable[] PROGMEM = "reachable";
const char key_recycle[] PROGMEM = "recycle";
const char key_sat[] PROGMEM = "sat";
const char key_sat_inc[] PROGMEM = "sat_inc";
const char key_sat_dec[] PROGMEM = "sat_dec";
const char key_scene[] PROGMEM = "scene";
const char key_sensors[] PROGMEM = "sensors";
const char key_state[] PROGMEM = "state";
const char key_storelightstate[] PROGMEM = "storelightstate";
const char key_transitiontime[] PROGMEM = "transitiontime";
const char key_type[] PROGMEM = "type";
const char key_uniqueid[] PROGMEM = "uniqueid";
const char key_username[] PROGMEM = "username";
// JSON values
const char value_none[] PROGMEM = "none";
const char value_active[] PROGMEM = "active";
// Enum Strings
const char unknown[] PROGMEM = "Unknown";
// Group::Type
const char luminaire[] PROGMEM = "Luminaire";
const char lightsource[] PROGMEM = "Lightsource";
const char light_group[] PROGMEM = "LightGroup";
const char room[] PROGMEM = "Room";
const char entertainment[] PROGMEM = "Entertainment";
const char zone[] PROGMEM = "Zone";
// Group::Class
const char living_room[] PROGMEM = "Living room";
const char kitchen[] PROGMEM = "Kitchen";
const char dining[] PROGMEM = "Dining";
const char bedroom[] PROGMEM = "Bedroom";
const char kids_bedroom[] PROGMEM = "Kids bedroom";
const char bathroom[] PROGMEM = "Bathroom";
const char nursery[] PROGMEM = "Nursery";
const char recreation[] PROGMEM = "Recreation";
const char office[] PROGMEM = "Office";
const char gym[] PROGMEM = "Gym";
const char hallway[] PROGMEM = "Hallway";
const char toilet[] PROGMEM = "Toilet";
const char front_door[] PROGMEM = "Front door";
const char garage[] PROGMEM = "Garage";
const char terrace[] PROGMEM = "Terrace";
const char garden[] PROGMEM = "Garden";
const char driveway[] PROGMEM = "Driveway";
const char carport[] PROGMEM = "Carport";
const char other[] PROGMEM = "Other";
const char home[] PROGMEM = "Home";
const char downstairs[] PROGMEM = "Downstairs";
const char upstairs[] PROGMEM = "Upstairs";
const char top_floor[] PROGMEM = "Top floor";
const char attic[] PROGMEM = "Attic";
const char guest_room[] PROGMEM = "Guest room";
const char staircase[] PROGMEM = "Staircase";
const char lounge[] PROGMEM = "Lounge";
const char man_cave[] PROGMEM = "Man cave";
const char computer[] PROGMEM = "Computer";
const char studio[] PROGMEM = "Studio";
const char music[] PROGMEM = "Music";
const char tv[] PROGMEM = "TV";
const char reading[] PROGMEM = "Reading";
const char closet[] PROGMEM = "Closet";
const char storage[] PROGMEM = "Storage";
const char laundry_room[] PROGMEM = "Laundry room";
const char balcony[] PROGMEM = "Balcony";
const char porch[] PROGMEM = "Porch";
const char barbecue[] PROGMEM = "Barbecue";
const char pool[] PROGMEM = "Pool";
// Scene::Type
const char light_scene[] PROGMEM = "LightScene";
const char group_scene[] PROGMEM = "GroupScene";
}

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

bool parseArrayOfIntStrings(json::JsonParser &parser, std::vector<uint8_t> &dest) {
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
// Class : DiscoveryResponse ///////////////////////////////////
////////////////////////////////////////////////////////////////

bool DiscoveryResponse::onKey(String &key, json::JsonParser &parser) {
  IF_EQ_RET(key, strings::key_id, parser.get(id_))
  IF_EQ_RET(key, strings::key_internalipaddress, parser.get(ip_))
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
  return (key == read_prog_str(strings::key_username)) ? parser.get(username_) : false;
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
  IF_EQ_RET(key, strings::key_on, parser.get(on_))
  IF_EQ_RET(key, strings::key_bri, parser.get(bri_))
  IF_EQ_RET(key, strings::key_hue, parser.get(hue_))
  IF_EQ_RET(key, strings::key_sat, parser.get(sat_))
  IF_EQ_RET(key, strings::key_ct, parser.get(ct_))
  IF_EQ_RET(key, strings::key_reachable, parser.get(reachable_))
  return false;
}


////////////////////////////////////////////////////////////////
// Class : Light ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

bool Light::onKey(String &key, json::JsonParser &parser) {
  IF_EQ_RET(key, strings::key_state, parser.get(state_))
  IF_EQ_RET(key, strings::key_name, parser.get(name_))
  IF_EQ_RET(key, strings::key_uniqueid, parser.get(uniqueid_))
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
  if (key == read_prog_str(strings::key_lastscan)) {
    String value;
    success = parser.get(value);
    if (success) {
      if (value == read_prog_str(strings::value_none)) {
        lastscan_ = 0;
      } else if (value == read_prog_str(strings::value_active)) {
        lastscan_ = -1;
      } else {
        success = timestampFromString(value, lastscan_);
      }
    }
  } else if (key == read_prog_str(strings::key_name)) {
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
  String key = read_prog_str(strings::key_on);
  add(key, turned_on);
}


void LightStateChange::setBrightness(uint8_t brightness) {
  String key = read_prog_str(strings::key_bri);
  add(key, (int) brightness);
}


void LightStateChange::setHue(uint16_t hue) {
  String key = read_prog_str(strings::key_hue);
  add(key, (int) hue);
}


void LightStateChange::setSaturation(uint8_t saturation) {
  String key = read_prog_str(strings::key_sat);
  add(key, (int) saturation);
}


void LightStateChange::setColorTemp(uint16_t color_temp) {
  String key = read_prog_str(strings::key_ct);
  add(key, (int) color_temp);
}


void LightStateChange::setTransitionTime(uint16_t time_in_tenths_of_seconds) {
  String key = read_prog_str(strings::key_transitiontime);
  add(key, (int) time_in_tenths_of_seconds);
}


void LightStateChange::incrementBrightness(uint8_t brightness_increment) {
  String key = read_prog_str(strings::key_bri_inc);
  add(key, (int) brightness_increment);
}


void LightStateChange::decrementBrightness(uint8_t brightness_decrement) {
  String key = read_prog_str(strings::key_bri_dec);
  add(key, (int) -brightness_decrement);
}


void LightStateChange::incrementSaturation(uint8_t saturation_increment) {
  String key = read_prog_str(strings::key_sat_inc);
  add(key, (int) saturation_increment);
}


void LightStateChange::decrementSaturation(uint8_t saturation_decrement) {
  String key = read_prog_str(strings::key_sat_dec);
  add(key, (int) -saturation_decrement);
}


void LightStateChange::incrementHue(uint16_t hue_increment) {
  String key = read_prog_str(strings::key_hue_inc);
  add(key, (int) hue_increment);
}


void LightStateChange::decrementHue(uint16_t hue_decrement) {
  String key = read_prog_str(strings::key_hue_dec);
  add(key, (int) -hue_decrement);
}


void LightStateChange::incrementColorTemp(uint16_t color_temp_increment) {
  String key = read_prog_str(strings::key_ct_inc);
  add(key, (int) color_temp_increment);
}


void LightStateChange::decrementColorTemp(uint16_t color_temp_decrement) {
  String key = read_prog_str(strings::key_ct_dec);
  add(key, (int) -color_temp_decrement);
}


////////////////////////////////////////////////////////////////
// Class : Group ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

Group::Type Group::typeFromString(String &string) {
  IF_EQ_RET(string, strings::luminaire, Type::LUMINAIRE)
  IF_EQ_RET(string, strings::lightsource, Type::LIGHTSOURCE)
  IF_EQ_RET(string, strings::light_group, Type::LIGHT_GROUP)
  IF_EQ_RET(string, strings::room, Type::ROOM)
  IF_EQ_RET(string, strings::entertainment, Type::ENTERTAINMENT)
  IF_EQ_RET(string, strings::zone, Type::ZONE)
  // else
  return Type::UNKNOWN;
}

String Group::typeToString(Group::Type &type) {
  switch (type) {
    CASE_RETURN(Type::LUMINAIRE, strings::luminaire);
    CASE_RETURN(Type::LIGHTSOURCE, strings::lightsource);
    CASE_RETURN(Type::LIGHT_GROUP, strings::light_group);
    CASE_RETURN(Type::ROOM, strings::room);
    CASE_RETURN(Type::ENTERTAINMENT, strings::entertainment);
    CASE_RETURN(Type::ZONE, strings::zone);
    default:
      return read_prog_str(strings::unknown);
  }
}

Group::Class Group::classFromString(String &string) {
  IF_EQ_RET(string, strings::living_room, Class::LIVING_ROOM)
  IF_EQ_RET(string, strings::kitchen, Class::KITCHEN)
  IF_EQ_RET(string, strings::dining, Class::DINING)
  IF_EQ_RET(string, strings::bedroom, Class::BEDROOM)
  IF_EQ_RET(string, strings::kids_bedroom, Class::KIDS_BEDROOM)
  IF_EQ_RET(string, strings::bathroom, Class::BATHROOM)
  IF_EQ_RET(string, strings::nursery, Class::NURSERY)
  IF_EQ_RET(string, strings::recreation, Class::RECREATION)
  IF_EQ_RET(string, strings::office, Class::OFFICE)
  IF_EQ_RET(string, strings::gym, Class::GYM)
  IF_EQ_RET(string, strings::hallway, Class::HALLWAY)
  IF_EQ_RET(string, strings::toilet, Class::TOILET)
  IF_EQ_RET(string, strings::front_door, Class::FRONT_DOOR)
  IF_EQ_RET(string, strings::garage, Class::GARAGE)
  IF_EQ_RET(string, strings::terrace, Class::TERRACE)
  IF_EQ_RET(string, strings::garden, Class::GARDEN)
  IF_EQ_RET(string, strings::driveway, Class::DRIVEWAY)
  IF_EQ_RET(string, strings::carport, Class::CARPORT)
  IF_EQ_RET(string, strings::other, Class::OTHER)
  IF_EQ_RET(string, strings::home, Class::HOME)
  IF_EQ_RET(string, strings::downstairs, Class::DOWNSTAIRS)
  IF_EQ_RET(string, strings::upstairs, Class::UPSTAIRS)
  IF_EQ_RET(string, strings::top_floor, Class::TOP_FLOOR)
  IF_EQ_RET(string, strings::attic, Class::ATTIC)
  IF_EQ_RET(string, strings::guest_room, Class::GUEST_ROOM)
  IF_EQ_RET(string, strings::staircase, Class::STAIRCASE)
  IF_EQ_RET(string, strings::lounge, Class::LOUNGE)
  IF_EQ_RET(string, strings::man_cave, Class::MAN_CAVE)
  IF_EQ_RET(string, strings::computer, Class::COMPUTER)
  IF_EQ_RET(string, strings::studio, Class::STUDIO)
  IF_EQ_RET(string, strings::music, Class::MUSIC)
  IF_EQ_RET(string, strings::tv, Class::TV)
  IF_EQ_RET(string, strings::reading, Class::READING)
  IF_EQ_RET(string, strings::closet, Class::CLOSET)
  IF_EQ_RET(string, strings::storage, Class::STORAGE)
  IF_EQ_RET(string, strings::laundry_room, Class::LAUNDRY_ROOM)
  IF_EQ_RET(string, strings::balcony, Class::BALCONY)
  IF_EQ_RET(string, strings::porch, Class::PORCH)
  IF_EQ_RET(string, strings::barbecue, Class::BARBECUE)
  IF_EQ_RET(string, strings::pool, Class::POOL)
  // else
  return Class::UNKNOWN;
}

String Group::classToString(Group::Class &a_class) {
  switch (a_class) {
    CASE_RETURN(Class::LIVING_ROOM, strings::living_room);
    CASE_RETURN(Class::KITCHEN, strings::kitchen);
    CASE_RETURN(Class::DINING, strings::dining);
    CASE_RETURN(Class::BEDROOM, strings::bedroom);
    CASE_RETURN(Class::KIDS_BEDROOM, strings::kids_bedroom);
    CASE_RETURN(Class::BATHROOM, strings::bathroom);
    CASE_RETURN(Class::NURSERY, strings::nursery);
    CASE_RETURN(Class::RECREATION, strings::recreation);
    CASE_RETURN(Class::OFFICE, strings::office);
    CASE_RETURN(Class::GYM, strings::gym);
    CASE_RETURN(Class::HALLWAY, strings::hallway);
    CASE_RETURN(Class::TOILET, strings::toilet);
    CASE_RETURN(Class::FRONT_DOOR, strings::front_door);
    CASE_RETURN(Class::GARAGE, strings::garage);
    CASE_RETURN(Class::TERRACE, strings::terrace);
    CASE_RETURN(Class::GARDEN, strings::garden);
    CASE_RETURN(Class::DRIVEWAY, strings::driveway);
    CASE_RETURN(Class::CARPORT, strings::carport);
    CASE_RETURN(Class::OTHER, strings::other);
    CASE_RETURN(Class::HOME, strings::home);
    CASE_RETURN(Class::DOWNSTAIRS, strings::downstairs);
    CASE_RETURN(Class::UPSTAIRS, strings::upstairs);
    CASE_RETURN(Class::TOP_FLOOR, strings::top_floor);
    CASE_RETURN(Class::ATTIC, strings::attic);
    CASE_RETURN(Class::GUEST_ROOM, strings::guest_room);
    CASE_RETURN(Class::STAIRCASE, strings::staircase);
    CASE_RETURN(Class::LOUNGE, strings::lounge);
    CASE_RETURN(Class::MAN_CAVE, strings::man_cave);
    CASE_RETURN(Class::COMPUTER, strings::computer);
    CASE_RETURN(Class::STUDIO, strings::studio);
    CASE_RETURN(Class::MUSIC, strings::music);
    CASE_RETURN(Class::TV, strings::tv);
    CASE_RETURN(Class::READING, strings::reading);
    CASE_RETURN(Class::CLOSET, strings::closet);
    CASE_RETURN(Class::STORAGE, strings::storage);
    CASE_RETURN(Class::LAUNDRY_ROOM, strings::laundry_room);
    CASE_RETURN(Class::BALCONY, strings::balcony);
    CASE_RETURN(Class::PORCH, strings::porch);
    CASE_RETURN(Class::BARBECUE, strings::barbecue);
    CASE_RETURN(Class::POOL, strings::pool);
    default:
      return read_prog_str(strings::unknown);
  }
}


const String &Group::name() const {
  return name_;
}


const std::vector<uint8_t> &Group::lights() const {
  return lights_;
}


const std::vector<uint8_t> &Group::sensors() const {
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
  IF_EQ_RET(key, strings::key_name, parser.get(name_))
  IF_EQ_RET(key, strings::key_lights, parseArrayOfIntStrings(parser, lights_))
  IF_EQ_RET(key, strings::key_sensors, parseArrayOfIntStrings(parser, sensors_))
  if (key == read_prog_str(strings::key_type)) {
    String type;
    bool success = parser.get(type);
    type_ = typeFromString(type);
    return success;
  }
  IF_EQ_RET(key, strings::key_state, parser.get(*this))
  IF_EQ_RET(key, strings::key_all_on, parser.get(all_on_))
  IF_EQ_RET(key, strings::key_any_on, parser.get(any_on_))
  IF_EQ_RET(key, strings::key_recycle, parser.get(recycle_))
  if (key == read_prog_str(strings::key_class)) {
    String a_class;
    bool success = parser.get(a_class);
    class_ = classFromString(a_class);
    return success;
  }
  IF_EQ_RET(key, strings::key_action, parser.get(action_))
  return false;
}


////////////////////////////////////////////////////////////////
// Class : GroupAttributeChange ////////////////////////////////
////////////////////////////////////////////////////////////////

void GroupAttributeChange::addLight(int light_id) {
  String light_string(light_id);
  json::JsonString value(light_string);
  lights_.add(value);
}


void GroupAttributeChange::removeLight(int light_id) {
  String light_string(light_id);
  json::JsonString value(light_string);
  lights_.remove(value);
}


void GroupAttributeChange::setName(String &name) {
  String key = read_prog_str(strings::key_name);
  add(key, name);
}


void GroupAttributeChange::setRoomClass(Group::Class a_class) {
  String key = read_prog_str(strings::key_class);
  String value = Group::classToString(a_class);
  add(key, value);
}


void GroupAttributeChange::build() {
  String key = read_prog_str(strings::key_lights);
  if (has(key)) {
    // TODO: Equality check before remove?
    remove(key);
  }
  add(key, lights_);
}


////////////////////////////////////////////////////////////////
// Class : GroupCreationRequest ////////////////////////////////
////////////////////////////////////////////////////////////////

void GroupCreationRequest::setType(Group::Type type) {
  String key = read_prog_str(strings::key_type);
  String type_string = Group::typeToString(type);
  add(key, type_string);
  if (type != Group::Type::ROOM) {
    key = read_prog_str(strings::key_class);
    if (has(key)) {
      remove(key);
    }
  }
}


void GroupCreationRequest::setRoomClass(Group::Class a_class) {
  // Set type to ROOM
  String key = read_prog_str(strings::key_type);
  Group::Type type = Group::Type::ROOM;
  String value = Group::typeToString(type);
  add(key, value);
  // Populate value of class
  key = read_prog_str(strings::key_class);
  value = Group::classToString(a_class);
  add(key, value);
}


////////////////////////////////////////////////////////////////
// Class : GroupStateChange ////////////////////////////////////
////////////////////////////////////////////////////////////////

void GroupStateChange::setScene(String &scene) {
  String key = read_prog_str(strings::key_scene);
  add(key, scene);
}


////////////////////////////////////////////////////////////////
// Class : Scene ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

Scene::Type Scene::typeFromString(String &string) {
  IF_EQ_RET(string, strings::light_scene, Type::LIGHT_SCENE)
  IF_EQ_RET(string, strings::group_scene, Type::GROUP_SCENE);
  // else
  return Type::UNKNOWN;
}

String Scene::typeToString(Scene::Type &type) {
  switch (type) {
    CASE_RETURN(Type::LIGHT_SCENE, strings::light_scene);
    CASE_RETURN(Type::GROUP_SCENE, strings::group_scene);
    default:
      return read_prog_str(strings::unknown);
  }
}

const String &Scene::name() const {
  return name_;
}

Scene::Type Scene::type() const {
  return type_;
}

uint8_t Scene::group() const {
  return group_;
}

const std::vector<uint8_t> &Scene::lights() const {
  return lights_;
}

bool Scene::recycle() const {
  return recycle_;
}

bool Scene::locked() const {
  return locked_;
}

bool Scene::onKey(String &key, json::JsonParser &parser) {
  IF_EQ_RET(key, strings::key_name, parser.get(name_))
  if (key == read_prog_str(strings::key_type)) {
    String type;
    bool success = parser.get(type);
    type_ = typeFromString(type);
    return success;
  }
  if (key == read_prog_str(strings::key_group)) {
    String group;
    bool success = parser.get(group);
    group_ = group.toInt();
    return success;
  }
  IF_EQ_RET(key, strings::key_lights, parseArrayOfIntStrings(parser, lights_))
  IF_EQ_RET(key, strings::key_recycle, parser.get(recycle_))
  IF_EQ_RET(key, strings::key_locked, parser.get(locked_))
  return false;
}

////////////////////////////////////////////////////////////////
// Class : SceneAttributeChange ////////////////////////////////
////////////////////////////////////////////////////////////////

void SceneAttributeChange::addLight(int light_id) {
  String light_string(light_id);
  json::JsonString value(light_string);
  lights_.add(value);
}


void SceneAttributeChange::removeLight(int light_id) {
  String light_string(light_id);
  json::JsonString value(light_string);
  lights_.remove(value);
}


void SceneAttributeChange::setName(String &name) {
  String key = read_prog_str(strings::key_name);
  add(key, name);
}


void SceneAttributeChange::setStoreLightState(bool store_light_state) {
  String key = read_prog_str(strings::key_storelightstate);
  add(key, store_light_state);
}


void SceneAttributeChange::build() {
  String key = read_prog_str(strings::key_lights);
  if (has(key)) {
    // TODO: Equality check before remove?
    remove(key);
  }
  add(key, lights_);
}


////////////////////////////////////////////////////////////////
// Class : SceneStateChange ////////////////////////////////////
////////////////////////////////////////////////////////////////

void SceneStateChange::setOn(bool turned_on) {
  String key = read_prog_str(strings::key_on);
  add(key, turned_on);
}


void SceneStateChange::setBrightness(uint8_t brightness) {
  String key = read_prog_str(strings::key_bri);
  add(key, (int) brightness);
}


void SceneStateChange::setHue(uint16_t hue) {
  String key = read_prog_str(strings::key_hue);
  add(key, (int) hue);
}


void SceneStateChange::setSaturation(uint8_t saturation) {
  String key = read_prog_str(strings::key_sat);
  add(key, (int) saturation);
}


void SceneStateChange::setColorTemp(uint16_t color_temp) {
  String key = read_prog_str(strings::key_ct);
  add(key, (int) color_temp);
}


void SceneStateChange::setTransitionTime(uint16_t time_in_tenths_of_seconds) {
  String key = read_prog_str(strings::key_transitiontime);
  add(key, (int) time_in_tenths_of_seconds);
}

}