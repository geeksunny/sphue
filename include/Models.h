#ifndef SPHUE_INCLUDE_MODELS_H_
#define SPHUE_INCLUDE_MODELS_H_

#include <JSON.h>

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
  const String getFullName() const;
  const String &getString() const;
  const int getInt() const;
  const float getFloat() const;
  const bool getBool() const;
 private:
  std::shared_ptr<String> prefix_;
  Type type_ = Type::UNKNOWN;
  String name_;
  String value_;
  bool onKey(String &key, json::JsonParser &parser) override;
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

class Lights : public json::JsonModel {
 public:
  std::map<int, Light> &operator*();
  const std::map<int, Light> &operator*() const;
 private:
  std::map<int, Light> lights_;
  bool onKey(String &key, json::JsonParser &parser) override;
};

class NewLights : public json::JsonModel {
 public:
  std::map<int, String> &operator*();
  const std::map<int, String> &operator*() const;
  long lastscan() const;
  bool isScanning() const;
 private:
  long lastscan_;
  std::map<int, String> lights_;
  int last_parsed_id_;
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

}

#endif //SPHUE_INCLUDE_MODELS_H_
