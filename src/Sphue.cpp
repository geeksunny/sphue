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

const char *endpoint_prefix = "api";

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

void Sphue::setInsecure() {
  client_.setRequireSelfSignedCert(false);
}

void Sphue::setRequireSelfSignedCert(bool require_self_signed_cert) {
  client_.setRequireSelfSignedCert(require_self_signed_cert);
}

void Sphue::setSslFingerprint(const char *fingerprint) {
  client_.setFingerprint(fingerprint);
}

template<typename T>
bool Sphue::parseSingleResponse(Stream &response_stream, Response<T> &dest) {
  json::JsonParser parser(response_stream);
  if (parser.findArray()) {
    json::JsonArrayIterator<Response<T>> array = parser.iterateArray<Response<T>>();
    return array.hasNext() && array.getNext(dest);
  } else {
    T response;
    bool success = parser.get(response);
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
  String endpoint = read_prog_str(strings::endpoint_lights);
  return get<Lights>(endpoint_prefix, apiKey_, endpoint.c_str());
}

Response<NewLights> Sphue::getNewLights() {
  String endpoint = read_prog_str(strings::endpoint_lights);
  return get<NewLights>(endpoint_prefix, apiKey_, endpoint.c_str(), "new");
}

Response<NamedValue> Sphue::searchForNewLights() {
  String endpoint = read_prog_str(strings::endpoint_lights);
  return post<NamedValue>(nullptr, endpoint_prefix, apiKey_, endpoint.c_str());
}

Response<Light> Sphue::getLight(int id) {
  String endpoint = read_prog_str(strings::endpoint_lights);
  return get<Light>(endpoint_prefix, apiKey_, endpoint.c_str(), id);
}

Response<NamedValue> Sphue::renameLight(int id, String &new_name) {
  json::JsonObject json;
  String key = "name";
  json.add(key, new_name);
  String endpoint = read_prog_str(strings::endpoint_lights);
  return post<NamedValue>(&json, endpoint_prefix, apiKey_, endpoint.c_str(), id);
}

std::vector<Response<NamedValue>> Sphue::setLightState(int id, LightStateChange &change) {
  String endpoint = read_prog_str(strings::endpoint_lights);
  return put(&change, endpoint_prefix, apiKey_, endpoint.c_str(), id, "state");
}

Response<String> Sphue::deleteLight(int id) {
  String endpoint = read_prog_str(strings::endpoint_lights);
  return del(endpoint_prefix, apiKey_, endpoint.c_str(), id);
}

Response<Groups> Sphue::getAllGroups() {
  String endpoint = read_prog_str(strings::endpoint_groups);
  return get<Groups>(endpoint_prefix, apiKey_, endpoint.c_str());
}

Response<NamedValue> Sphue::createGroup(GroupCreationRequest &request) {
  String endpoint = read_prog_str(strings::endpoint_groups);
  return post<NamedValue>(&request, endpoint_prefix, apiKey_, endpoint.c_str());
}

Response<Group> Sphue::getGroup(int id) {
  String endpoint = read_prog_str(strings::endpoint_groups);
  return get<Group>(endpoint_prefix, apiKey_, endpoint.c_str(), id);
}

std::vector<Response<NamedValue>> Sphue::setGroupAttributes(int id, GroupAttributeChange &change) {
  String endpoint = read_prog_str(strings::endpoint_groups);
  return post(&change, endpoint_prefix, apiKey_, endpoint.c_str(), id);
}

std::vector<Response<NamedValue>> Sphue::setGroupState(int id, GroupStateChange &change) {
  String endpoint = read_prog_str(strings::endpoint_groups);
  return put(&change, endpoint_prefix, apiKey_, endpoint.c_str(), id, "action");
}

Response<String> Sphue::deleteGroup(int id) {
  String endpoint = read_prog_str(strings::endpoint_groups);
  return del(endpoint_prefix, apiKey_, endpoint.c_str(), id);
}

Response<Scenes> Sphue::getAllScenes() {
  String endpoint = read_prog_str(strings::endpoint_scenes);
  return get<Scenes>(endpoint_prefix, apiKey_, endpoint.c_str());
}

Response<NamedValue> Sphue::createScene(SceneCreationRequest &request) {
  String endpoint = read_prog_str(strings::endpoint_scenes);
  return post<NamedValue>(&request, endpoint_prefix, apiKey_, endpoint.c_str());
}

Response<Scene> Sphue::getScene(int id) {
  String endpoint = read_prog_str(strings::endpoint_scenes);
  return get<Scene>(endpoint_prefix, apiKey_, endpoint.c_str(), id);
}

std::vector<Response<NamedValue>> Sphue::modifyScene(int id, SceneModificationRequest &change) {
  String endpoint = read_prog_str(strings::endpoint_scenes);
  return post(&change, endpoint_prefix, apiKey_, endpoint.c_str(), id);
}

Response<String> Sphue::deleteScene(int id) {
  String endpoint = read_prog_str(strings::endpoint_scenes);
  return del(endpoint_prefix, apiKey_, endpoint.c_str(), id);
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