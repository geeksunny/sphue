#include "Sphue.h"

#ifdef SPHUE_EXAMPLE_PROJECT
#include <iostream>
#endif

// Discovery
#define DISCOVER_ADDRESS                    "discovery.meethue.com"
#define DISCOVER_PORT                       443
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
  auto result = client.get("/");
  if (result.statusCode() == 200) {
    json::JsonParser parser(result);
    DiscoveryResponse response;
    json::JsonArrayIterator<DiscoveryResponse> array = parser.iterateArray<DiscoveryResponse>();
    while (array.hasNext()) {
      if (parser.get(response)) {
        Sphue sphue(response.ip().c_str());
        result.finish();
        return sphue;
      }
    }
  }
  result.finish();
  return Sphue(nullptr);
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

template<typename T>
bool Sphue::parseSingleResponse(Stream &response_stream, T &dest) {
  json::JsonParser parser(response_stream);
  if (parser.findArray()) {
    json::JsonArrayIterator<T> array = parser.iterateArray<T>();
    return array.hasNext() && parser.get(dest);
  } else {
    return parser.get(dest);
  }
}

template<typename T>
bool Sphue::parseFirstResponse(Stream &response_stream, T &dest) {
  json::JsonParser parser(response_stream);
  json::JsonArrayIterator<T> array = parser.iterateArray<T>();
  bool success = array.hasNext() && parser.get(dest);
  return success;
}

template<typename T>
std::vector<T> Sphue::parseResponses(Stream &response_stream, int size) {
  json::JsonParser parser(response_stream);
  std::vector<T> result(size);
  json::JsonArrayIterator<T> array = parser.iterateArray<T>();
  while (array.hasNext()) {
    T response;
    if (parser.get(response)) {
      result.push_back(response);
    }
  }
  array.finish();
  return result;
}

Response<RegisterResponse> Sphue::registerDeviceApiKey(const char *deviceName, const char *applicationName) {
  json::JsonObject json;
  // TODO: Consider possible refactors for JSON and HTTP client libraries for more efficient memory patterns.
  String key = String(CREATE_USER_KEY_DEVICETYPE);
  json.add(key, String(applicationName) + "#" + String(deviceName));
  auto result = client_.post(ENDPOINT_CREATE_USER, json.toJson().c_str());
  Response<RegisterResponse> response;
  parseFirstResponse(result, response);
  result.finish();
  return response;
}

}