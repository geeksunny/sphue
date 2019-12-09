#ifndef SPHUE_INCLUDE_MODELS_H_
#define SPHUE_INCLUDE_MODELS_H_

#include <JSON.h>
#include <vector>

namespace sphue {

class NamedValue : public json::JsonModel {
 public:
  NamedValue() = default;
  explicit NamedValue(std::shared_ptr<String> &prefix);
  enum Type {
    UNKNOWN,
    BOOL,
    INT,
    FLOAT,
    STRING
  };
  const String &getPrefix() const;
  Type getType() const;
  const String &getName() const;
  String getFullName() const;
  const String &getString() const;
  int getInt() const;
  float getFloat() const;
  bool getBool() const;
 private:
  std::shared_ptr<String> prefix_;
  Type type_ = Type::UNKNOWN;
  String name_;
  String value_;
  bool onKey(String &key, json::JsonParser &parser) override;
};

template<typename K, typename T>
class ParsedMap : public json::JsonModel {
 public:
  std::map<K, T> &operator*() {
    return values_;
  }
  const std::map<K, T> &operator*() const {
    return values_;
  }
 protected:
  std::map<K, T> values_;
  virtual inline K getKey(String &from) = 0;
 private:
  bool onKey(String &key, json::JsonParser &parser) override {
    T value;
    bool success = parser.get(value);
    values_[getKey(key)] = value;
    return success;
  }
};

template<typename T>
class ParsedIntMap : public ParsedMap<uint8_t, T> {
 protected:
  uint8_t getKey(String &from) override {
    return from.toInt();
  }
};

template<typename T>
class ParsedStringMap : public ParsedMap<String, T> {
 protected:
  String getKey(String &from) override {
    return from;
  }
};

template<typename T = json::JsonObject>
class BuildableObject : public T {
 public:
  String toJson() override {
    build();
    return T::toJson();
  }
 private:
  virtual void build() = 0;
};

class DiscoveryResponse : public json::JsonModel {
 public:
  const String &id() const;
  const String &ip() const;
 private:
  String id_;
  String ip_;
  bool onKey(String &key, json::JsonParser &parser) override;
};

class RegisterResponse : public json::JsonModel {
 public:
  const String &username() const;
 private:
  String username_;
  bool onKey(String &key, json::JsonParser &parser) override;
};

// TODO: Consider adding build flags for extended model fields, ie IFDEF SPHUE_RESPONSE_EXTENDED

// TODO: Should State be a nested class of Light?
class State : public json::JsonModel {
  // The following fields have been omitted for simplicity. They can be added in later if desired. //
  // String effect;
  // float xy[2];
  // String alert;
  // String colormode;
  // String mode;
 public:
  //
 private:
  bool on_;
  uint8_t bri_;
  uint16_t hue_;
  uint8_t sat_;
  uint8_t ct_;
  bool reachable_;
  bool onKey(String &key, json::JsonParser &parser) override;
};

// struct swupdate {} // needed?
// struct capabilities {} // needed?
// struct config {} // needed?

class Light : public json::JsonModel {
  // The following fields have been omitted for simplicity. They can be added in later if desired. //
  // String type;
  // String modelid;
  // String manufacturername;
  // String productname;
  // String swversion;
  // String swconfigid;
  // String productid;
 public:
  const State &state() const;
  const String &name() const;
  const String &uniqueid() const;
 private:
  State state_;
  String name_;
  String uniqueid_;
  bool onKey(String &key, json::JsonParser &parser) override;
};

typedef ParsedIntMap<Light> Lights;

class NewLights : public ParsedIntMap<String> {
 public:
  long lastscan() const;
  bool isScanning() const;
 private:
  long lastscan_ = 0;
  int last_parsed_id_ = 0;
  bool onKey(String &key, json::JsonParser &parser) override;
};

class LightStateChange : public json::JsonObject {
  // The following fields have been omitted for simplicity. They can be added in later if desired. //
  // float xy[2];
  // float xy_inc[2];
  // String alert;
  // String effect;
 public:
  void setOn(bool turned_on);
  void setBrightness(uint8_t brightness);
  void setHue(uint16_t hue);
  void setSaturation(uint8_t saturation);
  void setColorTemp(uint16_t color_temp);
  void setTransitionTime(uint16_t time_in_tenths_of_seconds);
  void incrementBrightness(uint8_t brightness_increment);
  void decrementBrightness(uint8_t brightness_decrement);
  void incrementSaturation(uint8_t saturation_increment);
  void decrementSaturation(uint8_t saturation_decrement);
  void incrementHue(uint16_t hue_increment);
  void decrementHue(uint16_t hue_decrement);
  void incrementColorTemp(uint16_t color_temp_increment);
  void decrementColorTemp(uint16_t color_temp_decrement);
};

class Group : public json::JsonModel {
 public:
  enum class Type {
    UNKNOWN,
    LUMINAIRE,
    LIGHTSOURCE,
    LIGHT_GROUP,
    ROOM,
    ENTERTAINMENT,
    ZONE
  };
  enum class Class {
    UNKNOWN,
    LIVING_ROOM,
    KITCHEN,
    DINING,
    BEDROOM,
    KIDS_BEDROOM,
    BATHROOM,
    NURSERY,
    RECREATION,
    OFFICE,
    GYM,
    HALLWAY,
    TOILET,
    FRONT_DOOR,
    GARAGE,
    TERRACE,
    GARDEN,
    DRIVEWAY,
    CARPORT,
    OTHER,
    HOME,
    DOWNSTAIRS,
    UPSTAIRS,
    TOP_FLOOR,
    ATTIC,
    GUEST_ROOM,
    STAIRCASE,
    LOUNGE,
    MAN_CAVE,
    COMPUTER,
    STUDIO,
    MUSIC,
    TV,
    READING,
    CLOSET,
    STORAGE,
    LAUNDRY_ROOM,
    BALCONY,
    PORCH,
    BARBECUE,
    POOL
  };
  static Type typeFromString(String &string) ICACHE_FLASH_ATTR;
  static String typeToString(Type &type) ICACHE_FLASH_ATTR;
  static Class classFromString(String &string) ICACHE_FLASH_ATTR;
  static String classToString(Class &a_class) ICACHE_FLASH_ATTR;
  const String &name() const;
  const std::vector<uint8_t> &lights() const;
  const std::vector<uint8_t> &sensors() const;
  bool allOn() const;
  bool anyOn() const;
  bool recycle() const;
  const State &action() const;
 private:
  String name_;
  std::vector<uint8_t> lights_;
  std::vector<uint8_t> sensors_;
  Type type_;
  bool all_on_;
  bool any_on_;
  bool recycle_;
  Class class_;
  State action_;
  bool onKey(String &key, json::JsonParser &parser) override;
};

typedef ParsedIntMap<Group> Groups;

class GroupAttributeChange : public BuildableObject<> {
 public:
  void addLight(int light_id);
  void removeLight(int light_id);
  void setName(String &name);
  virtual void setRoomClass(Group::Class a_class);
  void build() override;
 private:
  json::JsonArray<json::JsonString> lights_;
};

class GroupCreationRequest : public GroupAttributeChange {
 public:
  void setType(Group::Type type);
  void setRoomClass(Group::Class a_class) override;
};

class GroupStateChange : public LightStateChange {
 public:
  void setScene(String &scene);
};

class Scene : public json::JsonModel {
  // The following fields have been omitted for simplicity. They can be added in later if desired. //
  // String owner_;
  // appdata : { version: uint8_t, data: String }
  // String picture_;
  // long last_updated_;
  // uint8_t version_;
 public:
  enum class Type {
    UNKNOWN,
    LIGHT_SCENE,
    GROUP_SCENE
  };
  static Type typeFromString(String &string) ICACHE_FLASH_ATTR;
  static String typeToString(Type &type) ICACHE_FLASH_ATTR;
  const String &name() const;
  Type type() const;
  uint8_t group() const;
  const std::vector<uint8_t> &lights() const;
  bool recycle() const;
  bool locked() const;
 private:
  String name_;
  Type type_;
  uint8_t group_;
  std::vector<uint8_t> lights_;
  bool recycle_;
  bool locked_;
  bool onKey(String &key, json::JsonParser &parser) override;
};

typedef ParsedStringMap<Scene> Scenes;

class SceneCreationRequest : public json::JsonObject {
};

class SceneModificationRequest : public json::JsonObject {};

class SceneAttributeChange : public BuildableObject<SceneModificationRequest> {
 public:
  void addLight(int light_id);
  void removeLight(int light_id);
  void setName(String &name);
  // TODO: "lightstates": {"#":{lightstate_object}, ...}
  void setStoreLightState(bool store_light_state);
  void build() override;
 private:
  json::JsonArray<json::JsonString> lights_;
};

class SceneStateChange : public SceneModificationRequest {
  // The following fields have been omitted for simplicity. They can be added in later if desired. //
  // float xy[2];
  // String effect;
 public:
  void setOn(bool turned_on);
  void setBrightness(uint8_t brightness);
  void setHue(uint16_t hue);
  void setSaturation(uint8_t saturation);
  void setColorTemp(uint16_t color_temp);
  void setTransitionTime(uint16_t time_in_tenths_of_seconds);
};

}

#endif //SPHUE_INCLUDE_MODELS_H_
