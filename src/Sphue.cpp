#include "Sphue.h"
#include <sstream>
#include "prog_str.h"

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

namespace sphue {

namespace strings {
const char endpoint_lights[] PROGMEM = "lights";
const char endpoint_groups[] PROGMEM = "groups";
const char endpoint_scenes[] PROGMEM = "scenes";
}

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
  parseFirstResponse(result, response);
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
  READ_PROG_STR(strings::endpoint_lights, endpoint)
  return get<Lights>(endpoint);
}

Response<NewLights> Sphue::getNewLights() {
  READ_PROG_STR(strings::endpoint_lights, endpoint)
  return get<NewLights>(endpoint, "new");
}

Response<NamedValue> Sphue::searchForNewLights() {
  READ_PROG_STR(strings::endpoint_lights, endpoint)
  return post<NamedValue>(nullptr, endpoint);
}

Response<Light> Sphue::getLight(int id) {
  READ_PROG_STR(strings::endpoint_lights, endpoint)
  return get<Light>(endpoint, id);
}

Response<NamedValue> Sphue::renameLight(int id, String &new_name) {
  json::JsonObject json;
  String key = "name";
  json.add(key, new_name);
  READ_PROG_STR(strings::endpoint_lights, endpoint)
  return post<NamedValue>(&json, endpoint, id);
}

std::vector<Response<NamedValue>> Sphue::setLightState(int id, LightStateChange &change) {
  READ_PROG_STR(strings::endpoint_lights, endpoint)
  return put(&change, endpoint, id, "state");
}

Response<String> Sphue::deleteLight(int id) {
  READ_PROG_STR(strings::endpoint_lights, endpoint)
  return del(endpoint, id);
}

Response<Groups> Sphue::getAllGroups() {
  READ_PROG_STR(strings::endpoint_groups, endpoint)
  return get<Groups>(endpoint);
}

Response<NamedValue> Sphue::createGroup(GroupCreationRequest &request) {
  READ_PROG_STR(strings::endpoint_groups, endpoint)
  return post<NamedValue>(&request, endpoint);
}

Response<Group> Sphue::getGroup(int id) {
  READ_PROG_STR(strings::endpoint_groups, endpoint)
  return get<Group>(endpoint, id);
}

std::vector<Response<NamedValue>> Sphue::setGroupAttributes(int id, GroupAttributeChange &change) {
  READ_PROG_STR(strings::endpoint_groups, endpoint)
  return post(&change, endpoint, id);
}

std::vector<Response<NamedValue>> Sphue::setGroupState(int id, GroupStateChange &change) {
  READ_PROG_STR(strings::endpoint_groups, endpoint)
  return put(&change, endpoint, id, "action");
}

Response<String> Sphue::deleteGroup(int id) {
  READ_PROG_STR(strings::endpoint_groups, endpoint)
  return del(endpoint, id);
}

Response<Scenes> Sphue::getAllScenes() {
  READ_PROG_STR(strings::endpoint_scenes, endpoint)
  return get<Scenes>(endpoint);
}

Response<NamedValue> Sphue::createScene(SceneCreationRequest &request) {
  READ_PROG_STR(strings::endpoint_scenes, endpoint)
  return post<NamedValue>(&request, endpoint);
}

Response<Scene> Sphue::getScene(int id) {
  READ_PROG_STR(strings::endpoint_scenes, endpoint)
  return get<Scene>(endpoint, id);
}

std::vector<Response<NamedValue>> Sphue::modifyScene(int id, SceneModificationRequest &change) {
  READ_PROG_STR(strings::endpoint_scenes, endpoint)
  return post(&change, endpoint, id);
}

Response<String> Sphue::deleteScene(int id) {
  READ_PROG_STR(strings::endpoint_scenes, endpoint)
  return del(endpoint, id);
}

Response<RegisterResponse> Sphue::registerDeviceApiKey(const char *deviceName, const char *applicationName) {
  json::JsonObject json;
  // TODO: Consider possible refactors for JSON and HTTP client libraries for more efficient memory patterns.
  String key = String(CREATE_USER_KEY_DEVICETYPE);
  String value = String(applicationName) + "#" + String(deviceName);
  json.add(key, value);
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