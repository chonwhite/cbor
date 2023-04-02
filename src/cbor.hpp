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

using array_iterator = std::vector<DataItem>::const_iterator;
using map_iterator = std::map<DataItem, DataItem>::const_iterator;

class iterator {
public:
  struct detail {
    type_t type_;
    array_iterator array_iterator_;
    map_iterator map_iterator_;
  };

  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = DataItem;
  using pointer = const DataItem *;
  using reference = const DataItem &;

  iterator(iterator::detail detail) : detail_(detail) {}

  // for map iteration;
  reference key() const;
  reference value() const;

  reference operator*() const { return *detail_.array_iterator_; }
  pointer operator->() { return detail_.array_iterator_.operator->(); }

  iterator &operator++(); // Prefix increment
  iterator operator++(int); // Postfix increment

  friend bool operator==(const iterator &a, const iterator &b);
  friend bool operator!=(const iterator &a, const iterator &b);

private:
  iterator::detail detail_;
};

// TODO iterator<T> ?
class iterator_wrapper {
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = iterator;
  using pointer = const iterator *;
  using reference = const iterator &;

  iterator_wrapper(iterator begin, iterator end) : iter_(begin), end_(end) {}

  reference operator*() const { return iter_; }
  pointer operator->() { return &iter_; }

  iterator_wrapper begin() {
    return iterator_wrapper(iter_, end_);
  }

  iterator_wrapper end() {
    return iterator_wrapper(end_, end_);
  }
  
  iterator_wrapper &operator++() {
    iter_++;
    return *this;
  }
  iterator_wrapper operator++(int i) {
    iterator iter = iter_.operator++(i);
    return iterator_wrapper(iter, end_);
  }

  friend bool operator==(const iterator_wrapper &a, const iterator_wrapper &b) {
    return a.iter_ == b.iter_;
  }
  friend bool operator!=(const iterator_wrapper &a, const iterator_wrapper &b) {
    return a.iter_ != b.iter_;
  }

private:
  iterator iter_;
  iterator end_;
};

enum class stream_mode : uint8_t {
  Text,
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

  DataItem& at(size_t index);
  const DataItem& at(size_t index) const; 

  template <typename T> T as() { return (T) * this; }
  template <typename T> T get() { return (T) * this; }

  iterator begin() const noexcept;
  iterator end() const noexcept;
  iterator_wrapper items() const noexcept;

  uint64_t tag() const;
  DataItem child() const;

  bool read(std::istream &in);
  void write(std::ostream &out) const;

  /**
   * @brief Set output stream mode, Text mode can be useful when
   * outputing to std::cout.
   * @param mode 
   */
  void set_os_mode(stream_mode mode);

  void push_back(const DataItem &item);
  void push_back(DataItem &&item);

  template <class... Args> void emplace_back(Args &&...args) {
    type_ = type_t::Array;
    array_.emplace_back(std::forward<Args>(args)...);
  }
  // TODO at append, emplace ;

  bool is_empty() const;
  bool empty() const;

  size_t size() const;

  void clear();

  void operator=(const std::string &str);
  void operator=(const char *str);

  DataItem &operator[](const DataItem &key);
  DataItem &operator[](const DataItem &&key);

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

  std::string dump(int indent = 2) const;

  static DataItem tagged(unsigned long long tag, const DataItem &value);
  static bool validate(const std::vector<uint8_t> &in);

  friend std::istream& operator>> (std::istream& is, DataItem& item);
  friend std::ostream& operator<<(std::ostream& os, const DataItem& item);

  friend iterator;
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
  stream_mode output_mode_ = stream_mode::Text;

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
