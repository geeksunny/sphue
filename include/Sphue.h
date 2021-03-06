#ifndef SPHUE_INCLUDE_SPHUE_H_
#define SPHUE_INCLUDE_SPHUE_H_

#include "Models.h"
#include <Rested.h>
#include "PgmStringTools.hpp"

#define SPHUE_APP_NAME        "Sphue"

namespace sphue {

namespace strings {
// JSON keys
const char key_success[] PROGMEM = "success";
const char key_error[] PROGMEM = "error";
// `key_type` is duplicated in Models.cpp - can they be combined easily to save space?
const char key_type[] PROGMEM = "type";
const char key_address[] PROGMEM = "address";
const char key_description[] PROGMEM = "description";
}

namespace ResultCode {
const uint16_t UNKNOWN                          = 0;
const uint16_t UNAUTHORIZED                     = 1;
const uint16_t INVALID_JSON                     = 2;
const uint16_t RESOURCE_UNAVAILABLE             = 3;
const uint16_t METHOD_UNAVAILABLE               = 4;
const uint16_t MISSING_PARAMS                   = 5;
const uint16_t PARAM_UNAVAILABLE                = 6;
const uint16_t INVALID_PARAM_VALUE              = 7;
const uint16_t PARAM_READ_ONLY                  = 8;
const uint16_t TOO_MANY_LIST_ITEMS              = 11;
const uint16_t PORTAL_CONNECTION_REQUIRED       = 12;
const uint16_t HUB_BUTTON_NOT_PRESSED           = 101;
const uint16_t DHCP_CANNOT_BE_DISABLED          = 110;
const uint16_t INVALID_UPDATESTATE              = 111;
const uint16_t OK                               = 200;
const uint16_t DEVICE_IS_OFF                    = 201;
const uint16_t LIGHT_LIST_FULL                  = 203;
const uint16_t GROUP_LIST_FULL                  = 301;
const uint16_t CANNOT_DELETE_GROUP              = 305;
const uint16_t LIGHT_ALREADY_IN_LIST            = 306;
const uint16_t ERROR                            = 400;
const uint16_t SCENE_LIST_FULL                  = 402;
const uint16_t SCENE_IS_LOCKED                  = 403;
const uint16_t SCENE_GROUP_EMPTY                = 404;
const uint16_t CANNOT_CREATE_SENSOR             = 501;
const uint16_t SENSOR_LIST_FULL                 = 502;
const uint16_t COMMISIONABLE_SENSOR_LIST_FULL   = 503;
const uint16_t RULE_LIST_FULL                   = 601;
const uint16_t CONDITION_ERROR                  = 607;
const uint16_t ACTION_ERROR                     = 608;
const uint16_t CANNOT_ACTIVATE                  = 609;
const uint16_t SCHEDULE_LIST_FULL               = 701;
const uint16_t TIMEZONE_NOT_VALID               = 702;
const uint16_t CANNOT_SET_BOTH_TIMES            = 703;
const uint16_t CANNOT_CREATE_SCHEDULE           = 704;
const uint16_t SCHEDULE_EXPIRED                 = 705;
const uint16_t COMMAND_ERROR                    = 706;
const uint16_t SOURCE_MODEL_INVALID             = 801;
const uint16_t SOURCE_FACTORY_NEW               = 802;
const uint16_t INVALID_STATE                    = 803;
}

template<typename T>
class Response : public json::JsonModel {
  friend class Sphue;

 public:
  explicit Response() = default;

  T &operator*() {
    return result_;
  }

  const T &operator*() const {
    return result_;
  }

  explicit operator bool() {
    return result_code_ == ResultCode::OK;
  }

  explicit operator bool() const {
    return result_code_ == ResultCode::OK;
  }

  uint16_t resultCode() const {
    return result_code_;
  }

  const String &errorAddress() const {
    return error_address_;
  }

  const String &errorDescription() const {
    return error_description_;
  }

 private:
  uint16_t result_code_ = ResultCode::UNKNOWN;
  String error_address_;
  String error_description_;
  T result_;

  explicit Response(T &result) : result_code_(ResultCode::OK), result_(result) {
    //
  }

  bool onKey(String &key, json::JsonParser &parser) override {
    STR_EQ_INIT(key.c_str())
    STR_EQ_RET(strings::key_success, parser.get(result_))
    STR_EQ_RET(strings::key_error, parser.get(*this))
    STR_EQ_RET(strings::key_type, parser.get(result_code_))
    STR_EQ_RET(strings::key_address, parser.get(error_address_))
    STR_EQ_RET(strings::key_description, parser.get(error_description_))
    return false;
  }
};

class Sphue {
 public:
  explicit Sphue(const char *apiKey, const char *hostname, int port = 80);
  explicit Sphue(const char *hostname, int port = 80);

  const char *getApiKey();
  void setApiKey(const char *apiKey);

  void setInsecure();
  void setRequireSelfSignedCert(bool require_self_signed_cert);
  void setSslFingerprint(const char *fingerprint);

  // Lights API
  Response<Lights> getAllLights();
  Response<NewLights> getNewLights();
  Response<NamedValue> searchForNewLights();
  Response<Light> getLight(int id);
  Response<NamedValue> renameLight(int id, String &new_name);
  std::vector<Response<NamedValue>> setLightState(int id, LightStateChange &change);
  Response<String> deleteLight(int id);

  // Groups API
  Response<Groups> getAllGroups();
  Response<NamedValue> createGroup(GroupCreationRequest &request);
  Response<Group> getGroup(int id);
  std::vector<Response<NamedValue>> setGroupAttributes(int id, GroupAttributeChange &change);
  std::vector<Response<NamedValue>> setGroupState(int id, GroupStateChange &change);
  Response<String> deleteGroup(int id);

  // Scenes API
  Response<Scenes> getAllScenes();
  Response<NamedValue> createScene(SceneCreationRequest &request);
  Response<Scene> getScene(int id);
  std::vector<Response<NamedValue>> modifyScene(int id, SceneModificationRequest &change);
  Response<String> deleteScene(int id);

  // Configuration API
  Response<RegisterResponse> registerDeviceApiKey(const char *deviceName, const char *applicationName = SPHUE_APP_NAME);

  // TODO : Implement other APIs?
  //  - Schedules, https://developers.meethue.com/develop/hue-api/3-schedules-api/
  //  - Sensors, https://developers.meethue.com/develop/hue-api/5-sensors-api/
  //  - Rules, https://developers.meethue.com/develop/hue-api/6-rules-api/
  //  - Configuration, https://developers.meethue.com/develop/hue-api/7-configuration-api/
  //  - Resource links, https://developers.meethue.com/develop/hue-api/9-resourcelinks-api/
  //  - Capabilities, https://developers.meethue.com/develop/hue-api/10-capabilities-api/

  explicit operator bool();
  explicit operator bool() const;

 private:
  // TODO: Is StreamedSecureRestClient suitable for HTTP (non-SSL) hosts?
  rested::StreamedSecureRestClient client_;
  const char *apiKey_;

  template<typename T>
  bool parseSingleResponse(Stream &response_stream, Response<T> &dest);
  template<typename T>
  bool parseFirstResponse(Stream &response_stream, Response<T> &dest);
  template<typename T>
  std::vector<Response<T>> parseResponses(Stream &response_stream, int size = 0);

  String buildEndpoint(std::stringstream &string_builder);
  template<typename T, typename... Args>
  String buildEndpoint(std::stringstream &string_builder, T &value, Args&... args);
  template<typename... Args>
  String makeEndpoint(Args&... args);

  template<typename T, typename... Endpoint>
  Response<T> get(Endpoint... args);
  template<typename T, typename... Endpoint>
  Response<T> post(json::JsonObject *body, Endpoint... args);
  template<typename... Endpoint>
  std::vector<Response<NamedValue>> post(json::JsonObject *body, Endpoint... args);
  template<typename... Endpoint>
  std::vector<Response<NamedValue>> put(json::JsonObject *body, Endpoint... args);
  template<typename... Endpoint>
  Response<String> del(Endpoint... args);
};

Sphue autoDiscoverHub(const char *hubId = nullptr);

}

#endif //SPHUE_INCLUDE_SPHUE_H_
