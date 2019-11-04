#include "Sphue.h"

#ifdef SPHUE_EXAMPLE_PROJECT
#include <iostream>
#endif

// Discovery
#define DISCOVER_ADDRESS                    "discovery.meethue.com"
#define DISCOVER_PORT                       443
// API Endpoints
// - Create User
#define ENDPOINT_LIGHTS                     "/lights/"
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
      if (array.getNext(response)) {
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
bool Sphue::parseSingleResponse(Stream &response_stream, Response<T> &dest) {
  json::JsonParser parser(response_stream);
  if (parser.findArray()) {
    json::JsonArrayIterator<Response<T>> array = parser.iterateArray<Response<T>>();
    return array.hasNext() && array.getNext(dest);
  } else {
    T response;
    bool success = parser.get(dest);
    dest = Response<T>(response);
    return success;
  }
}

template<typename T>
bool Sphue::parseFirstResponse(Stream &response_stream, Response<T> &dest) {
  json::JsonParser parser(response_stream);
  json::JsonArrayIterator<Response<T>> array = parser.iterateArray<Response<T>>();
  bool success = array.hasNext() && array.getNext(dest);
  return success;
}

template<typename T>
std::vector<Response<T>> Sphue::parseResponses(Stream &response_stream, int size) {
  json::JsonParser parser(response_stream);
  std::vector<Response<T>> result(size);
  json::JsonArrayIterator<Response<T>> array = parser.iterateArray<Response<T>>();
  while (array.hasNext()) {
    Response<T> response;
    if (array.getNext(response)) {
      result.push_back(response);
    }
  }
  return result;
}

Response<Lights> Sphue::getAllLights() {
  auto result = client_.get(ENDPOINT_LIGHTS);
  Response<Lights> response;
  parseSingleResponse(result, response);
  result.finish();
  return response;
}

Response<NewLights> Sphue::getNewLights() {
  String endpoint = ENDPOINT_LIGHTS + String("new");
  auto result = client_.get(endpoint.c_str());
  Response<NewLights> response;
  parseSingleResponse(result, response);
  result.finish();
  return response;
}

Response<NamedValue> Sphue::searchForNewLights() {
  auto result = client_.post(ENDPOINT_LIGHTS, "");
  Response<NamedValue> response;
  parseFirstResponse(result, response);
  result.finish();
  return response;
}

Response<Light> Sphue::getLight(int id) {
  String endpoint = ENDPOINT_LIGHTS + String(id);
  auto result = client_.get(endpoint.c_str());
  Response<Light> response;
  parseSingleResponse(result, response);
  result.finish();
  return response;
}

Response<NamedValue> Sphue::renameLight(int id, String &new_name) {
  json::JsonObject json;
  String key = "name";
  json.add(key, new_name);
  String endpoint = ENDPOINT_LIGHTS + String(id);
  auto result = client_.post(endpoint.c_str(), json.toJson().c_str());
  Response<NamedValue> response;
  parseSingleResponse(result, response);
  result.finish();
  return response;
}

std::vector<Response<NamedValue>> Sphue::setLightState(int id, LightStateChange &change) {
  String endpoint = ENDPOINT_LIGHTS + String(id) + "/state";
  auto result = client_.put(endpoint.c_str(), change.toJson().c_str());
  std::vector<Response<NamedValue>> response = parseResponses<NamedValue>(result, change.size());
  result.finish();
  return response;
}

Response<String> Sphue::deleteLight(int id) {
  String endpoint = ENDPOINT_LIGHTS + String(id);
  auto result = client_.del(endpoint.c_str());
  Response<String> response;
  parseFirstResponse(result, response);
  result.finish();
  return response;
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