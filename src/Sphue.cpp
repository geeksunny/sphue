#include "Sphue.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#ifdef SPHUE_EXAMPLE_PROJECT
#include <iostream>
#endif

#define DISCOVER_ADDRESS    "discovery.meethue.com"
#define DISCOVER_PORT       443
#define KEY_ID              "id"
#define KEY_IP_ADDRESS      "internalipaddress"

namespace sphue {

Sphue autoDiscoverHub() {
  RestClient client(DISCOVER_ADDRESS, DISCOVER_PORT, 1);
  StaticJsonDocument<256> doc;
  RestResponse *result = client.getStreamed("/");
  int resultCode = result->status();
  auto error = deserializeJson(doc, *result);
#ifdef SPHUE_EXAMPLE_PROJECT
  std::cout << "N-UPnP result code: " << resultCode << " | boolean: " << (*result ? "TRUE" : "FALSE") << std::endl
            << "JSON Error? " << (error ? "YES!" : "NO") << std::endl;
  if (!error) {
    JsonArray resultArray = doc.as<JsonArray>();
    std::cout << "JSON Array Size : " << resultArray.size() << std::endl;
    if (resultArray.size() > 0) {
      JsonObject record = resultArray.getElement(0).as<JsonObject>();
      std::cout << "first id found : " << (const char *) record[KEY_ID] << " | first IP found : "
                << (const char *) record[KEY_IP_ADDRESS] << std::endl;
    }
  }
#endif
  result->finish();
  if (resultCode == HTTP_CODE_OK) {
//    auto error = ARDUINOJSON_NAMESPACE::deserializeJson(json, result);
//    if (!error) {
//      return Sphue(nullptr);
//    }
  }
  return nullptr;
}

Sphue::Sphue(const char *hostname) : client_(hostname) {
  //
}

Sphue::Sphue(const char *hostname, int port) : client_(hostname, port) {
  //
}

}