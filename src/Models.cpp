#include "Models.h"

namespace sphue {

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


const String &RegisterResponse::username() const {
  return username_;
}


bool RegisterResponse::onKey(String &key, json::JsonParser &parser) {
  return (key == "username") ? parser.get(username_) : false;
}


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

}