#ifndef SPHUE_INCLUDE_MODELS_H_
#define SPHUE_INCLUDE_MODELS_H_

#include <JSON.h>

namespace sphue {

 class DiscoveryResponse : public json::JsonModel {
  public:
   const String &id() const;
   const String &ip() const;
  private:
   String id_;
   String ip_;

   bool onKey(String &key, json::JsonParser &parser) override;
 };

}

#endif //SPHUE_INCLUDE_MODELS_H_
