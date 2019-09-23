#include "Sphue.h"
#include <ArduinoJson.h>

#ifdef SPHUE_EXAMPLE_PROJECT
#include <iostream>
#endif

#define DISCOVER_ADDRESS    "discovery.meethue.com"
#define DISCOVER_PORT       443
#define KEY_ID              "id"
#define KEY_IP_ADDRESS      "internalipaddress"

namespace sphue {

Sphue autoDiscoverHub(const char *hubId) {
  rested::StreamedSecureRestClient client(DISCOVER_ADDRESS, DISCOVER_PORT);
  StaticJsonDocument<256> doc;
  auto result = client.get("/");
  int resultCode = result.statusCode();
  auto error = deserializeJson(doc, result);
#ifdef SPHUE_EXAMPLE_PROJECT
  std::cout << "N-UPnP result code: " << resultCode << " | boolean: " << (result ? "TRUE" : "FALSE") << std::endl
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
  result.finish();
  if (resultCode == 200) {
//    auto error = ARDUINOJSON_NAMESPACE::deserializeJson(json, result);
//    if (!error) {
//      return Sphue(nullptr);
//    }
  }
//  return nullptr;
  return Sphue(nullptr);
}

Sphue::Sphue(const char *hostname) : client_(hostname) {
  //
}

Sphue::Sphue(const char *hostname, int port) : client_(hostname, port) {
  //
}

}