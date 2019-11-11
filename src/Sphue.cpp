#include "Sphue.h"
#include <sstream>

#ifdef SPHUE_EXAMPLE_PROJECT
#include <iostream>
#endif

// Discovery
#define DISCOVER_ADDRESS                    "discovery.meethue.com"
#define DISCOVER_PORT                       443
// API Endpoints
// - Create User
#define ENDPOINT_LIGHTS                     "lights"
#define ENDPOINT_GROUPS                     "groups"
#define ENDPOINT_CREATE_USER                "/api"
#define CREATE_USER_KEY_DEVICETYPE          "devicetype"

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

String Sphue::buildEndpoint(std::stringstream &string_builder) {
  return String(string_builder.str().c_str());
}

template<typename T, typename... Args>
String Sphue::buildEndpoint(std::stringstream &string_builder, T &value, Args &... args) {
  string_builder << '/' << value;
  return buildEndpoint(string_builder, args...);
}

template<typename... Args>
String Sphue::makeEndpoint(Args &... args) {
  std::stringstream ss;
  return buildEndpoint(ss, args...);
}

template<typename T, typename... Endpoint>
Response<T> Sphue::get(Endpoint... args) {
  auto result = client_.get(makeEndpoint(args...).c_str());
  Response<T> response;
  parseSingleResponse(result, response);
  result.finish();
  return response;
}

template<typename T, typename... Endpoint>
Response<T> Sphue::post(json::JsonObject *body, Endpoint... args) {
  auto result = client_.post(makeEndpoint(args...).c_str(), (body ? body->toJson().c_str() : ""));
  Response<T> response;
//  parseFirstResponse(result, response);
  parseSingleResponse(result, response);
  result.finish();
  return response;
}

template<typename... Endpoint>
std::vector<Response<NamedValue>> Sphue::post(json::JsonObject *body, Endpoint... args) {
  auto result = client_.post(makeEndpoint(args...).c_str(), (body ? body->toJson().c_str() : ""));
  std::vector<Response<NamedValue>> response = parseResponses<NamedValue>(result, (body ? body->size() : 0));
  result.finish();
  return response;
}

template<typename... Endpoint>
std::vector<Response<NamedValue>> Sphue::put(json::JsonObject *body, Endpoint... args) {
  auto result = client_.put(makeEndpoint(args...).c_str(), (body ? body->toJson().c_str() : ""));
  std::vector<Response<NamedValue>> response = parseResponses<NamedValue>(result, (body ? body->size() : 0));
  result.finish();
  return response;
}

template<typename... Endpoint>
Response<String> Sphue::del(Endpoint... args) {
  auto result = client_.del(makeEndpoint(args...).c_str());
  Response<String> response;
  parseFirstResponse(result, response);
  result.finish();
  return response;
}

Response<Lights> Sphue::getAllLights() {
  return get<Lights>(ENDPOINT_LIGHTS);
}

Response<NewLights> Sphue::getNewLights() {
  return get<NewLights>(ENDPOINT_LIGHTS, "new");
}

Response<NamedValue> Sphue::searchForNewLights() {
  return post<NamedValue>(nullptr, ENDPOINT_LIGHTS);
}

Response<Light> Sphue::getLight(int id) {
  return get<Light>(ENDPOINT_LIGHTS, id);
}

Response<NamedValue> Sphue::renameLight(int id, String &new_name) {
  json::JsonObject json;
  String key = "name";
  json.add(key, new_name);
  return post<NamedValue>(&json, ENDPOINT_LIGHTS, id);
}

std::vector<Response<NamedValue>> Sphue::setLightState(int id, LightStateChange &change) {
  return put(&change, ENDPOINT_LIGHTS, id, "state");
}

Response<String> Sphue::deleteLight(int id) {
  return del(ENDPOINT_LIGHTS, id);
}

Response<Groups> Sphue::getAllGroups() {
  return get<Groups>(ENDPOINT_GROUPS);
}

Response<NamedValue> Sphue::createGroup(GroupCreationRequest &request) {
  return post<NamedValue>(&request, ENDPOINT_GROUPS);
}

Response<Group> Sphue::getGroup(int id) {
  return get<Group>(ENDPOINT_GROUPS, id);
}

std::vector<Response<NamedValue>> Sphue::setGroupAttributes(int id, GroupAttributeChange &change) {
  return post(&change, ENDPOINT_GROUPS, id);
}

std::vector<Response<NamedValue>> Sphue::setGroupState(int id, GroupStateChange &change) {
  return put(&change, ENDPOINT_GROUPS, id, "action");
}

Response<String> Sphue::deleteGroup(int id) {
  return del(ENDPOINT_GROUPS, id);
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

Sphue::operator bool() {
  return (bool) client_;
}

Sphue::operator bool() const {
  return (bool) client_;
}

}