#ifndef SPHUE_INCLUDE_MODELS_H_
#define SPHUE_INCLUDE_MODELS_H_

#include <JSON.h>

namespace sphue {

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

}

#endif //SPHUE_INCLUDE_MODELS_H_
