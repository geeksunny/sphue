#include "Sphue.h"
#include <ArduinoJson.h>

#ifdef SPHUE_EXAMPLE_PROJECT
#include <iostream>
#endif

// Discovery
#define DISCOVER_ADDRESS                    "discovery.meethue.com"
#define DISCOVER_PORT                       443
#define KEY_ID                              "id"
#define KEY_IP_ADDRESS                      "internalipaddress"
// API Endpoints
// - Create User
#define ENDPOINT_CREATE_USER                "/api"
#define CREATE_USER_KEY_DEVICETYPE          "devicetype"
// - General ...
#define ENDPOINT_PREFIX                     "/api/"
#define ENDPOINT_GET_CONFIG                 "/config"

namespace sphue {

inline const char *copyCStr(const char *str) {
  char *copy = new char[strlen(str) + 1]{};
  return std::copy(str, str + strlen(str), copy);
}

Sphue autoDiscoverHub(const char *hubId) {
  rested::StreamedSecureRestClient client(DISCOVER_ADDRESS, DISCOVER_PORT);
  StaticJsonDocument<256> doc;
  auto result = client.get("/");
  int resultCode = result.statusCode();
  if (resultCode == 200) {
    auto error = deserializeJson(doc, result);
    result.finish();
    if (!error) {
      JsonArray resultArray = doc.as<JsonArray>();
      if (resultArray.size() > 0) {
        JsonObject record = resultArray.getElement(0).as<JsonObject>();
        // TODO: Should we do something with the ID value?
        if (record.containsKey(KEY_IP_ADDRESS)) {
          return Sphue(copyCStr((const char *) record[KEY_IP_ADDRESS]));
        }
      }
      return Sphue(nullptr);
    }
  } else {
    result.finish();
    return Sphue(nullptr);
  }
}

Sphue::Sphue(const char *apiKey, const char *hostname, int port) : Sphue(hostname, port) {
  setApiKey(apiKey);
}

Sphue::Sphue(const char *hostname, int port) : client_(hostname, port) {
  //
}

const char *Sphue::getApiKey() {
  return apiKey_;
}

void Sphue::setApiKey(const char *apiKey) {
  apiKey_ = apiKey;
}

Result Sphue::registerDeviceApiKey() {
  StaticJsonDocument<128> body;
  //  <application_name>#<device_name>
  body[CREATE_USER_KEY_DEVICETYPE] = "Lightswitch#switch_control";
  auto response = client_.post(ENDPOINT_CREATE_USER, "");
  return Result();
}

}