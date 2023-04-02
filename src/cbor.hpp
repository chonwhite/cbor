#pragma once

#include <initializer_list>
#include <iostream>
#include <map>
#include <stdint.h>
#include <vector>

namespace cbor {

class DataItem;
using Array = std::vector<DataItem>;
using Map = std::vector<DataItem, DataItem>;

enum simple { // TODO
  False = 20,
  True,
  Null,
  Undefined,
  null = Null,
  undefined = Undefined
};

enum class type_t : uint8_t {
  Unsigned,
  Negative,
  String,
  Array,
  Map,
  Tagged,
  Simple,
  Float,
  Binary,
};

class DataItem {
public:
  DataItem(std::nullptr_t);
  DataItem(bool value);

  DataItem(uint8_t value);
  DataItem(uint16_t value);
  DataItem(uint32_t value);
  DataItem(uint64_t value);

  DataItem(int8_t value);
  DataItem(int16_t value);
  DataItem(int32_t value);
  DataItem(int64_t value);

  DataItem(float value);
  DataItem(double value);

  DataItem(const char *value);
  DataItem(const std::vector<uint8_t> &value);
  DataItem(const std::string &value);

  DataItem(const std::vector<DataItem> &value);
  DataItem(const std::map<DataItem, DataItem> &value);
  DataItem(simple value = simple::Undefined);

  type_t type() const;

  bool is_unsigned() const;
  bool is_signed() const;
  bool is_int() const;
  bool is_binary() const;
  bool is_string() const;
  bool is_array() const;
  bool is_map() const;
  bool is_tagged() const;
  bool is_simple() const;
  bool is_bool() const;
  bool is_null() const;
  bool is_undefined() const;
  bool is_float() const;
  bool is_number() const;

  template <typename T> T as() { return (T) * this; }
  template <typename T> T get() {return (T) * this; }

  uint64_t tag() const;
  DataItem child() const;

  bool read(std::istream &in);
  void write(std::ostream &out) const;

  void push_back(const DataItem &item);
  void push_back(DataItem &&item);

  template <class... Args>
  void emplace_back(Args &&...args) {
    type_ = type_t::Array;
    array_.emplace_back(std::forward<Args>(args)...);
  }

  // TODO at append, emplace clear;

  bool is_empty() const;
  bool empty() const;

  size_t size() const;

  void operator=(const std::string &str);
  void operator=(const char *str);

  DataItem &operator[](const DataItem &key);
  DataItem &operator[](const char *key);

  operator uint8_t() const;
  operator uint16_t() const;
  operator uint32_t() const;
  operator uint64_t() const;

  operator bool() const;
  operator int8_t() const;
  operator int16_t() const;
  operator int32_t() const;
  operator int64_t() const;

  operator float() const;
  operator double() const;

  operator std::vector<uint8_t>() const;
  operator std::string() const;
  operator std::vector<DataItem>() const;
  operator std::map<DataItem, DataItem>() const;
  operator cbor::simple() const;

  bool operator==(const DataItem &other) const;
  bool operator!=(const DataItem &other) const;

  bool operator<(const DataItem &other) const;

  // TODO stream operator << and >>;
  // TODO iterator;

  static DataItem tagged(unsigned long long tag, const DataItem &value);
  static bool validate(const std::vector<uint8_t> &in);
  static std::string debug(const DataItem &in);

private:
  cbor::type_t type_ = type_t::Simple; // TODO null;
  union {
    uint64_t value_;
    double float_;
  };
  std::vector<uint8_t> binary_;
  std::string string_;
  std::vector<DataItem> array_;
  std::map<DataItem, DataItem> map_;

  uint64_t to_unsigned() const;
  int64_t to_signed() const;
  double to_float() const;

  std::vector<uint8_t> to_binary() const;
  std::string to_string() const;
  std::vector<DataItem> to_array() const;
  std::map<DataItem, DataItem> to_map() const;
  simple to_simple() const;
};

DataItem decode(std::vector<uint8_t> &binary);
void encode(DataItem &item, std::vector<uint8_t> &binary);

DataItem array(std::initializer_list<DataItem> items = {});
DataItem map(std::initializer_list<std::pair<DataItem, DataItem>> items = {});

} // namespace cbor
