#ifndef SPHUE_INCLUDE_SPHUE_H_
#define SPHUE_INCLUDE_SPHUE_H_

#include <Rested.h>

namespace sphue {

class Sphue {
  rested::StreamedBasicRestClient client_;
 public:
  Sphue(const char *hostname);
  Sphue(const char *hostname, int port);
  // TODO: Constructor WifiClient to feed into RestClient if desired
};

Sphue autoDiscoverHub(const char *hubId = nullptr);

}

#endif //SPHUE_INCLUDE_SPHUE_H_
