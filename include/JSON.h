#ifndef SPHUE_INCLUDE_JSON_H_
#define SPHUE_INCLUDE_JSON_H_

#include <Stream.h>
#include <deque>
#include <map>

namespace json {

class JsonSerializable {
 public:
  virtual String toJson() = 0;
};


class JsonString : public JsonSerializable {
  String value_;
 public:
  explicit JsonString(String &value);
  const String &getValue() const;
  void setValue(String &value);
  String toJson() override;
};


class JsonBool : public JsonSerializable {
  bool value_;
 public:
  explicit JsonBool(bool value);
  bool getValue();
  void setValue(bool value);
  String toJson() override;
};


enum JsonNumberType {
  INT,
  DOUBLE,
  FLOAT
};


class JsonNumber : public JsonSerializable {
  JsonNumberType type_;
  int int_value_;
  double double_value_;
  float float_value_;
 public:
  explicit JsonNumber(int value);
  explicit JsonNumber(double value);
  explicit JsonNumber(float value);

  int getInt();
  double getDouble();
  float getFloat();

  void setValue(int value);
  void setValue(double value);
  void setValue(float value);
  void setType(JsonNumberType type);

  String toJson() override;
};


template<typename SerializableType>
class JsonArray : public JsonSerializable {
  std::deque<SerializableType> values_;
 public:
  JsonArray() = default;
  explicit JsonArray(int initial_capacity);
  void add(SerializableType &value);
  bool remove(SerializableType &value);
  String toJson() override;
};


class JsonObject : public JsonSerializable {
  std::map<String, JsonSerializable> values_;
 public:
  JsonObject() = default;
  void add(String &key, JsonSerializable value);
  void add(String &key, String &value);
  void add(String &key, bool value);
  void add(String &key, int value);
  void add(String &key, double value);
  void add(String &key, float value);
  bool remove(String &key);
  bool has(String &key);
  String toJson() override;
};

}

#endif //SPHUE_INCLUDE_JSON_H_
