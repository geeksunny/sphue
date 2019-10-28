#ifndef SPHUE_INCLUDE_SPHUE_H_
#define SPHUE_INCLUDE_SPHUE_H_

#include "Models.h"
#include <Rested.h>


#define SPHUE_APP_NAME        "Sphue"

namespace sphue {

enum ResultCode {
  ERROR_400,
  OK_200,
  HUB_BUTTON_NOT_PRESSED_101
};

template<typename T>
class Response : public json::JsonModel {
 public:
  T &operator*() {
    return result_;
  }

  const T &operator*() const {
    return result_;
  }

  explicit operator bool() {
    return result_code_ != OK_200;
  }

  explicit operator bool() const {
    return result_code_ != OK_200;
  }

  ResultCode resultCode() const {
    return result_code_;
  }

  const String &errorAddress() const {
    return error_address_;
  }

  const String &errorDescription() const {
    return error_description_;
  }

 private:
  ResultCode result_code_;
  String error_address_;
  String error_description_;
  T result_;

  bool onKey(String &key, json::JsonParser &parser) override {
    if (key == "success") {
      return parser.get(result_);
    } else if (key == "error") {
      return parser.get(*this);
    } else if (key == "type") {
      // TODO: parse value for result_code_
    } else if (key == "address") {
      return parser.get(error_address_);
    } else if (key == "description") {
      return parser.get(error_description_);
    }
    return false;
  }
};

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

};

Sphue autoDiscoverHub(const char *hubId = nullptr);

}

#endif //SPHUE_INCLUDE_SPHUE_H_
