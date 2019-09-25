#ifndef SPHUE_INCLUDE_SPHUE_H_
#define SPHUE_INCLUDE_SPHUE_H_

#include <Rested.h>

namespace sphue {

enum ResultCode {
  ERROR_400,
  OK_200,
  HUB_BUTTON_NOT_PRESSED_101
};

struct Result {
  ResultCode resultCode;
  String result;
  // TODO: bool equality operator to check against resultCode for OK_200 (+ maybe others)
};

class Sphue {
 public:
  explicit Sphue(const char *apiKey, const char *hostname, int port = 80);
  explicit Sphue(const char *hostname, int port = 80);

  const char *getApiKey();
  void setApiKey(const char *apiKey);

  Result registerDeviceApiKey();

 private:
  rested::StreamedBasicRestClient client_;
  const char *apiKey_;

};

Sphue autoDiscoverHub(const char *hubId = nullptr);

}

#endif //SPHUE_INCLUDE_SPHUE_H_
