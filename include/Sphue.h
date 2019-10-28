#ifndef SPHUE_INCLUDE_SPHUE_H_
#define SPHUE_INCLUDE_SPHUE_H_

#include "Models.h"
#include <Rested.h>


#define SPHUE_APP_NAME        "Sphue"

namespace sphue {

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
 public:
  explicit Response() = default;

  T &operator*() {
    return result_;
  }

  const T &operator*() const {
    return result_;
  }

  explicit operator bool() {
    return result_code_ != ResultCode::OK;
  }

  explicit operator bool() const {
    return result_code_ != ResultCode::OK;
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

 private:
  explicit Response(T &result) : result_(result), result_code_(ResultCode::OK) {
    //
  }

  bool onKey(String &key, json::JsonParser &parser) override {
    if (key == "success") {
      return parser.get(result_);
    } else if (key == "error") {
      return parser.get(*this);
    } else if (key == "type") {
      return parser.get(result_code_);
    } else if (key == "address") {
      return parser.get(error_address_);
    } else if (key == "description") {
      return parser.get(error_description_);
    }
    return false;
  }
};

class Sphue {
 public:
  explicit Sphue(const char *apiKey, const char *hostname, int port = 80);
  explicit Sphue(const char *hostname, int port = 80);

  const char *getApiKey();
  void setApiKey(const char *apiKey);

  Response<RegisterResponse> registerDeviceApiKey(const char *deviceName, const char *applicationName = SPHUE_APP_NAME);

 private:
  rested::StreamedBasicRestClient client_;
  const char *apiKey_;

  template<typename T>
  bool parseSingleResponse(Stream &response_stream, Response<T> &dest);
  template<typename T>
  bool parseFirstResponse(Stream &response_stream, Response<T> &dest);
  template<typename T>
  std::vector<Response<T>> parseResponses(Stream &response_stream, int size = 0);
};

Sphue autoDiscoverHub(const char *hubId = nullptr);

}

#endif //SPHUE_INCLUDE_SPHUE_H_
