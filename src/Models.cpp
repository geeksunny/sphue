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

}