#include "Models.h"
#include <ctime>
#include "PgmStringTools.hpp"
#include "EnumTools.hpp"

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


String NamedValue::getFullName() const {
  if (prefix_) {
    return *prefix_.get() + name_;
  } else {
    return name_;
  }
}


const String &NamedValue::getString() const {
  return value_;
}


int NamedValue::getInt() const {
  return value_.toInt();
}


float NamedValue::getFloat() const {
  return value_.toFloat();
}


bool NamedValue::getBool() const {
  return value_ == "1";
}


////////////////////////////////////////////////////////////////
// Class : DiscoveryResponse ///////////////////////////////////
////////////////////////////////////////////////////////////////

bool DiscoveryResponse::onKey(String &key, json::JsonParser &parser) {
  STR_EQ_INIT(key.c_str())
  STR_EQ_RET(strings::key_id, parser.get(id_))
  STR_EQ_RET(strings::key_internalipaddress, parser.get(ip_))
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
  STR_EQ_INIT(key.c_str())
  STR_EQ_RET(strings::key_on, parser.get(on_))
  STR_EQ_RET(strings::key_bri, parser.get(bri_))
  STR_EQ_RET(strings::key_hue, parser.get(hue_))
  STR_EQ_RET(strings::key_sat, parser.get(sat_))
  STR_EQ_RET(strings::key_ct, parser.get(ct_))
  STR_EQ_RET(strings::key_reachable, parser.get(reachable_))
  return false;
}


bool State::on() const {
  return on_;
}


uint8_t State::bri() const {
  return bri_;
}


uint16_t State::hue() const {
  return hue_;
}


uint8_t State::sat() const {
  return sat_;
}


uint8_t State::ct() const {
  return ct_;
}


bool State::reachable() const {
  return reachable_;
}


////////////////////////////////////////////////////////////////
// Class : Light ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

bool Light::onKey(String &key, json::JsonParser &parser) {
  STR_EQ_INIT(key.c_str())
  STR_EQ_RET(strings::key_state, parser.get(state_))
  STR_EQ_RET(strings::key_name, parser.get(name_))
  STR_EQ_RET(strings::key_uniqueid, parser.get(uniqueid_))
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

MAKE_ENUM_MAP(group_type_map, Group::Type,
              MAPPING(Group::Type::LUMINAIRE, strings::luminaire),
              MAPPING(Group::Type::LIGHTSOURCE, strings::lightsource),
              MAPPING(Group::Type::LIGHT_GROUP, strings::light_group),
              MAPPING(Group::Type::ROOM, strings::room),
              MAPPING(Group::Type::ENTERTAINMENT, strings::entertainment),
              MAPPING(Group::Type::ZONE, strings::zone)
)

Group::Type Group::typeFromString(String &string) {
  return pgm_string_to_enum(string.c_str(), Type::UNKNOWN, group_type_map);
}

String Group::typeToString(Group::Type &type) {
  return enum_to_pgm_string(type, PGM_STR_AND_SIZE(strings::unknown), group_type_map);
}

MAKE_ENUM_MAP(group_class_map, Group::Class,
              MAPPING(Group::Class::LIVING_ROOM, strings::living_room),
              MAPPING(Group::Class::KITCHEN, strings::kitchen),
              MAPPING(Group::Class::DINING, strings::dining),
              MAPPING(Group::Class::BEDROOM, strings::bedroom),
              MAPPING(Group::Class::KIDS_BEDROOM, strings::kids_bedroom),
              MAPPING(Group::Class::BATHROOM, strings::bathroom),
              MAPPING(Group::Class::NURSERY, strings::nursery),
              MAPPING(Group::Class::RECREATION, strings::recreation),
              MAPPING(Group::Class::OFFICE, strings::office),
              MAPPING(Group::Class::GYM, strings::gym),
              MAPPING(Group::Class::HALLWAY, strings::hallway),
              MAPPING(Group::Class::TOILET, strings::toilet),
              MAPPING(Group::Class::FRONT_DOOR, strings::front_door),
              MAPPING(Group::Class::GARAGE, strings::garage),
              MAPPING(Group::Class::TERRACE, strings::terrace),
              MAPPING(Group::Class::GARDEN, strings::garden),
              MAPPING(Group::Class::DRIVEWAY, strings::driveway),
              MAPPING(Group::Class::CARPORT, strings::carport),
              MAPPING(Group::Class::OTHER, strings::other),
              MAPPING(Group::Class::HOME, strings::home),
              MAPPING(Group::Class::DOWNSTAIRS, strings::downstairs),
              MAPPING(Group::Class::UPSTAIRS, strings::upstairs),
              MAPPING(Group::Class::TOP_FLOOR, strings::top_floor),
              MAPPING(Group::Class::ATTIC, strings::attic),
              MAPPING(Group::Class::GUEST_ROOM, strings::guest_room),
              MAPPING(Group::Class::STAIRCASE, strings::staircase),
              MAPPING(Group::Class::LOUNGE, strings::lounge),
              MAPPING(Group::Class::MAN_CAVE, strings::man_cave),
              MAPPING(Group::Class::COMPUTER, strings::computer),
              MAPPING(Group::Class::STUDIO, strings::studio),
              MAPPING(Group::Class::MUSIC, strings::music),
              MAPPING(Group::Class::TV, strings::tv),
              MAPPING(Group::Class::READING, strings::reading),
              MAPPING(Group::Class::CLOSET, strings::closet),
              MAPPING(Group::Class::STORAGE, strings::storage),
              MAPPING(Group::Class::LAUNDRY_ROOM, strings::laundry_room),
              MAPPING(Group::Class::BALCONY, strings::balcony),
              MAPPING(Group::Class::PORCH, strings::porch),
              MAPPING(Group::Class::BARBECUE, strings::barbecue),
              MAPPING(Group::Class::POOL, strings::pool)
)

Group::Class Group::classFromString(String &string) {
  return pgm_string_to_enum(string.c_str(), Class::UNKNOWN, group_class_map);
}

String Group::classToString(Group::Class &a_class) {
  return enum_to_pgm_string(a_class, PGM_STR_AND_SIZE(strings::unknown), group_class_map);
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
  STR_EQ_INIT(key.c_str())
  STR_EQ_RET(strings::key_name, parser.get(name_))
  STR_EQ_RET(strings::key_lights, parseArrayOfIntStrings(parser, lights_))
  STR_EQ_RET(strings::key_sensors, parseArrayOfIntStrings(parser, sensors_))
  STR_EQ_DO(strings::key_type, {
    String type;
    bool success = parser.get(type);
    type_ = typeFromString(type);
    return success;
  })
  STR_EQ_RET(strings::key_state, parser.get(*this))
  STR_EQ_RET(strings::key_all_on, parser.get(all_on_))
  STR_EQ_RET(strings::key_any_on, parser.get(any_on_))
  STR_EQ_RET(strings::key_recycle, parser.get(recycle_))
  STR_EQ_DO(strings::key_class, {
    String a_class;
    bool success = parser.get(a_class);
    class_ = classFromString(a_class);
    return success;
  })
  STR_EQ_RET(strings::key_action, parser.get(action_))
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

MAKE_ENUM_MAP(scene_type_map, Scene::Type,
              MAPPING(Scene::Type::LIGHT_SCENE, strings::light_scene),
              MAPPING(Scene::Type::GROUP_SCENE, strings::group_scene)
)

Scene::Type Scene::typeFromString(String &string) {
  return pgm_string_to_enum(string.c_str(), Scene::Type::UNKNOWN, scene_type_map);
}

String Scene::typeToString(Scene::Type &type) {
  return enum_to_pgm_string(type, PGM_STR_AND_SIZE(strings::unknown), scene_type_map);
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
  STR_EQ_INIT(key.c_str())
  STR_EQ_RET(strings::key_name, parser.get(name_))
  STR_EQ_DO(strings::key_type, {
    String type;
    bool success = parser.get(type);
    type_ = typeFromString(type);
    return success;
  })
  STR_EQ_DO(strings::key_group, {
    String group;
    bool success = parser.get(group);
    group_ = group.toInt();
    return success;
  })
  STR_EQ_RET(strings::key_lights, parseArrayOfIntStrings(parser, lights_))
  STR_EQ_RET(strings::key_recycle, parser.get(recycle_))
  STR_EQ_RET(strings::key_locked, parser.get(locked_))
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