#ifndef SPHUE_INCLUDE_SPHUE_H_
#define SPHUE_INCLUDE_SPHUE_H_

#include <RestClient.h>

namespace sphue {

class Sphue {
  RestClient client_;
 public:
  Sphue(const char *hostname);
  Sphue(const char *hostname, int port);
  // TODO: Constructor WifiClient to feed into RestClient if desired
};

Sphue autoDiscoverHub();
Sphue autoDiscoverHub(String &hubId);

}

#endif //SPHUE_INCLUDE_SPHUE_H_
