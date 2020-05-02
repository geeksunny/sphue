#ifndef SPHUE_INCLUDE_JSON_H_
#define SPHUE_INCLUDE_JSON_H_

#include <Stream.h>
#include <memory>
#include <deque>
#include <map>

namespace json {

int strToInt(String &value);


class JsonParser;


template<typename T>
class JsonArrayIterator;


enum JsonValueType {
  INVALID,
  STRING,
  NUMBER,
  OBJECT,
  ARRAY,
  BOOL,
  NUL
};


enum JsonNumberType {
  INT,
  DOUBLE,
  FLOAT
};


class JsonModel {
  friend class JsonParser;

 private:
  virtual bool onKey(String &key, JsonParser &parser) = 0;
};


class JsonParser {
  friend class JsonModel;

 public:
  explicit JsonParser(Stream &src);

  bool get(JsonModel &dest);
  bool get(bool &dest);
  bool get(int &dest);
  bool get(uint8_t &dest) {
    return get((int &) dest);
  }
  bool get(uint16_t &dest) {
    return get((int &) dest);
  }
  bool get(double &dest);
  bool get(float &dest) {
    return get((double &) dest);
  }
  bool get(String &dest);
  //bool getHexString(int &dest);

  bool getAsString(String &dest);

  bool findChar(char find, bool skipWhitespace = true);
  bool findChar(char find, char skipChar, bool skipWhitespace = true);
  bool findChar(char find, const char *skipChars, bool skipWhitespace = true);

  JsonValueType checkValueType();
  JsonValueType checkValueType(char firstChar);

  bool findArray();
  bool findObject();

  bool peekMatches(char c);
  bool readMatches(char c);
  bool readMatches(const char *value, bool case_sensitive = true);
  bool skipValue();

  template<typename T>
  JsonArrayIterator<T> iterateArray() {
    return JsonArrayIterator<T>(*this);
  }

 private:
  Stream &src_;

  bool findNextKey(String &dest);
  bool findValue();

  bool getDigits(int &dest, bool allow_sign);
  bool getExponent(double &dest);
  bool getPrecision(double &dest);
};


template<typename T>
class JsonArrayIterator {
 public:
  explicit JsonArrayIterator(JsonParser &parser) : parser_(parser) {
    hasNext_ = parser_.findChar('[') && parser_.readMatches('[') && !parser_.findChar(']');
  }

  bool hasNext() {
    return hasNext_;
  }

  bool getNext(T &dest) {
    bool success = parser_.get(dest);
    hasNext_ = parser_.findChar(',') && parser_.readMatches(',');
    return success;
  }

  bool finish() {
    return parser_.skipValue() && parser_.readMatches(']');
  }

 private:
  JsonParser &parser_;
  bool hasNext_;
};


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
  bool operator==(const JsonString &rhs) const;
  bool operator!=(const JsonString &rhs) const;
};


class JsonBool : public JsonSerializable {
  bool value_;
 public:
  explicit JsonBool(bool value);
  bool getValue();
  void setValue(bool value);
  String toJson() override;
  bool operator==(const JsonBool &rhs) const;
  bool operator!=(const JsonBool &rhs) const;
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

  bool operator==(const JsonNumber &rhs) const;
  bool operator!=(const JsonNumber &rhs) const;
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
  bool operator==(const JsonArray &rhs) const;
  bool operator!=(const JsonArray &rhs) const;
};


class JsonObject : public JsonSerializable {
  std::map<String, std::unique_ptr<JsonSerializable>> values_;
 public:
  JsonObject() = default;

  template<typename T>
  void add(String &key, T &value);
  void add(String &key, String &value);
  void add(String &key, bool value);
  void add(String &key, int value);
  void add(String &key, double value);
  void add(String &key, float value);
  bool remove(String &key);
  bool has(String &key);
  int size();
  String toJson() override;
  bool operator==(const JsonObject &rhs) const;
  bool operator!=(const JsonObject &rhs) const;
};


}

#endif //SPHUE_INCLUDE_JSON_H_
