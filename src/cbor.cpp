#include "cbor.hpp"
#include <cmath>
#include <cstdio>
#include <sstream>

namespace cbor {

namespace major {
const int Unsigned = 0;
const int Negative = 1;
const int ByteString = 2;
const int TextString = 3;
const int Array = 4;
const int Map = 5;
const int Tag = 6;
const int Simple = 7; // float and simple;
} // namespace major

DataItem decode(const std::vector<uint8_t> &in) {
  std::istringstream iss(std::string(in.begin(), in.end()));
  DataItem item;
  if (item.read(iss) && iss.peek() == EOF) {
    return item;
  }
  return DataItem();
}

std::vector<uint8_t> encode(const DataItem &in) {
  std::ostringstream oss;
  in.write(oss);
  std::string buffer = oss.str();
  return std::vector<uint8_t>(buffer.begin(), buffer.end());
}

DataItem array(std::initializer_list<DataItem> items) {
  return DataItem(std::vector<DataItem>(items));
}

DataItem map(std::initializer_list<std::pair<DataItem, DataItem>> items) {
  DataItem item = DataItem(std::map<DataItem, DataItem>());
  for (auto it = items.begin(); it < items.end(); it++) {
    item[it->first] = it->second;
  }
  return item;
}

iterator::reference iterator::key() const {
  return detail_.map_iterator_->first;
}
iterator::reference iterator::value() const {
  return detail_.map_iterator_->second;
}

iterator &iterator::operator++() {
  if (detail_.type_ == type_t::Array) {
    detail_.array_iterator_++;
  }
  if (detail_.type_ == type_t::Map) {
    detail_.map_iterator_++;
  }
  return *this;
}

// Postfix increment
iterator iterator::operator++(int i) {
  iterator::detail tmp;
  if (detail_.type_ == type_t::Array) {
    tmp.array_iterator_ = detail_.array_iterator_.operator++(i);
  }
  if (detail_.type_ == type_t::Map) {
    tmp.map_iterator_ = detail_.map_iterator_.operator++(i);
  }
  return iterator(tmp);
}

bool operator==(const iterator &a, const iterator &b) {
  if (a.detail_.type_ != b.detail_.type_) {
    return false;
  };
  if (a.detail_.type_ == type_t::Array) {
    return a.detail_.array_iterator_ == b.detail_.array_iterator_;
  }
  if (a.detail_.type_ == type_t::Map) {
    return a.detail_.map_iterator_ == b.detail_.map_iterator_;
  }
  return false;
};

bool operator!=(const iterator &a, const iterator &b) {
  return !(operator==(a, b));
};

DataItem::DataItem(std::nullptr_t)
    : type_(type_t::Simple), value_(simple::Null) {}

DataItem::DataItem(bool value)
    : type_(type_t::Simple), value_(value ? simple::True : simple::False) {}

DataItem::DataItem(uint8_t value) : type_(type_t::Unsigned), value_(value) {}

DataItem::DataItem(uint16_t value) : type_(type_t::Unsigned), value_(value) {}

DataItem::DataItem(uint32_t value) : type_(type_t::Unsigned), value_(value) {}

DataItem::DataItem(uint64_t value) : type_(type_t::Unsigned), value_(value) {}

DataItem::DataItem(int8_t value)
    : type_(value < 0 ? type_t::Negative : type_t::Unsigned),
      value_(value < 0 ? -1 - value : value) {}

DataItem::DataItem(int16_t value)
    : type_(value < 0 ? type_t::Negative : type_t::Unsigned),
      value_(value < 0 ? -1 - value : value) {}

DataItem::DataItem(int32_t value)
    : type_(value < 0 ? type_t::Negative : type_t::Unsigned),
      value_(value < 0 ? -1 - value : value) {}

DataItem::DataItem(int64_t value)
    : type_(value < 0 ? type_t::Negative : type_t::Unsigned),
      value_(value < 0 ? -1 - value : value) {}

DataItem::DataItem(float value) : type_(type_t::Float), float_(value) {}
DataItem::DataItem(double value) : type_(type_t::Float), float_(value) {}

DataItem::DataItem(const std::vector<uint8_t> &value)
    : type_(type_t::Binary), binary_(value) {}

DataItem::DataItem(const std::string &value)
    : type_(type_t::String), string_(value) {}

DataItem::DataItem(const char *value) : type_(type_t::String), string_(value) {}

DataItem::DataItem(const std::vector<DataItem> &value)
    : type_(type_t::Array), array_(value) {}

DataItem::DataItem(const std::map<DataItem, DataItem> &value)
    : type_(type_t::Map), map_(value) {}

DataItem DataItem::tagged(unsigned long long tag, const DataItem &value) {
  DataItem result;
  result.type_ = type_t::Tagged;
  result.value_ = tag;
  result.array_ = std::vector<DataItem>(1, value);
  return result;
}
DataItem::DataItem(cbor::simple value)
    : type_(type_t::Simple), value_(value & 255) {}

bool DataItem::is_unsigned() const { return this->type_ == type_t::Unsigned; }
bool DataItem::is_signed() const {
  return (this->type_ == type_t::Unsigned || this->type_ == type_t::Negative) &&
         (this->value_ >> 63) == 0;
}
bool DataItem::is_int() const {
  return this->type_ == type_t::Unsigned || this->type_ == type_t::Negative;
}
bool DataItem::is_binary() const { return this->type_ == type_t::Binary; }
bool DataItem::is_string() const { return this->type_ == type_t::String; }
bool DataItem::is_array() const { return this->type_ == type_t::Array; }
bool DataItem::is_map() const { return this->type_ == type_t::Map; }
bool DataItem::is_tagged() const { return this->type_ == type_t::Tagged; }
bool DataItem::is_simple() const { return this->type_ == type_t::Simple; }
bool DataItem::is_bool() const {
  return this->type_ == type_t::Simple &&
         (this->value_ == simple::False || this->value_ == simple::True);
}
bool DataItem::is_null() const {
  return this->type_ == type_t::Simple && this->value_ == simple::Null;
}
bool DataItem::is_undefined() const {
  return this->type_ == type_t::Simple && this->value_ == simple::Undefined;
}
bool DataItem::is_float() const { return this->type_ == type_t::Float; }
bool DataItem::is_number() const {
  return this->type_ == type_t::Unsigned || this->type_ == type_t::Negative ||
         this->type_ == type_t::Float;
}

DataItem &DataItem::at(size_t index) { return array_.at(index); }

const DataItem &DataItem::at(size_t index) const { return array_.at(index); }

cbor::type_t DataItem::type() const { return this->type_; }
uint64_t DataItem::tag() const {
  switch (this->type_) {
  case type_t::Tagged:
    return this->value_;
  default:
    return 0;
  }
}
DataItem DataItem::child() const {
  switch (this->type_) {
  case type_t::Tagged:
    return this->array_.front();
  default:
    return DataItem();
  }
}

void DataItem::push_back(const DataItem &item) {
  type_ = type_t::Array;
  array_.push_back(item);
}

void DataItem::push_back(DataItem &&item) {
  type_ = type_t::Array;
  array_.push_back(item);
}

// template< class... Args >
// void DataItem::emplace_back( Args&&... args ) {
//   type_ = type_t::Array;
//   array_.emplace_back(std::forward<Args>(args)...);
// }

bool DataItem::is_empty() const {
  switch (type_) {
  case type_t::Array:
    return array_.empty();
  case type_t::Map:
    return map_.empty();
  // TODO tagged?
  case type_t::Simple:
    return this->value_ == simple::Null;
  default:
    return false;
  }
}

bool DataItem::empty() const { return is_empty(); }

size_t DataItem::size() const {
  switch (type_) {
  case type_t::Array:
    return array_.size();
  // TODO tagged?
  case type_t::Map:
    return map_.size();
  default:
    return 0; // TODO
  }
}

void DataItem::clear() {
  array_.clear();
  map_.clear();
}

iterator DataItem::begin() const noexcept {
  iterator::detail detail;
  detail.type_ = type_;
  detail.array_iterator_ = array_.begin();
  detail.map_iterator_ = map_.begin();
  return iterator(detail);
}

iterator DataItem::end() const noexcept {
  iterator::detail detail;
  detail.type_ = type_;
  detail.array_iterator_ = array_.end();
  detail.map_iterator_ = map_.end();
  return iterator(detail);
}

iterator_wrapper DataItem::items() const noexcept {
  return iterator_wrapper(begin(), end());
}
// iterator end() const noexcept;

/** ----------------- freinds -------------------- */

std::istream &operator>>(std::istream &is, DataItem &item) {
  item.read(is);
  return is;
}

std::ostream &operator<<(std::ostream &os, const DataItem &item) {
  if (item.output_mode_ == stream_mode::Binary) {
    item.write(os);
  } else {
    os << item.dump();
  }
  return os;
}

/** ----------------- private -------------------- */
uint64_t DataItem::to_unsigned() const {
  switch (this->type_) {
  case type_t::Unsigned:
    return this->value_;
  case type_t::Negative:
    return ~this->value_;
  case type_t::Tagged:
    return this->array_.front().to_unsigned();
  case type_t::Float:
    return this->float_;
  default:
    return 0;
  }
}
int64_t DataItem::to_signed() const {
  switch (this->type_) {
  case type_t::Unsigned:
    return int64_t(this->value_);
  case type_t::Negative:
    return -1 - int64_t(this->value_);
  case type_t::Tagged:
    return this->array_.front().to_signed();
  case type_t::Float:
    return this->float_;
  default:
    return 0;
  }
}
std::vector<uint8_t> DataItem::to_binary() const {
  switch (this->type_) {
  case type_t::Binary:
    return this->binary_;
  case type_t::Tagged:
    return this->array_.front().to_binary();
  default:
    return std::vector<uint8_t>();
  }
}
std::string DataItem::to_string() const {
  switch (this->type_) {
  case type_t::String:
    return this->string_;
  case type_t::Tagged:
    return this->array_.front().to_string();
  default:
    return std::string();
  }
}
std::vector<DataItem> DataItem::to_array() const {
  switch (this->type_) {
  case type_t::Array:
    return this->array_;
  case type_t::Tagged:
    return this->array_.front().to_array();
  default:
    return std::vector<DataItem>();
  }
}
std::map<DataItem, DataItem> DataItem::to_map() const {
  switch (this->type_) {
  case type_t::Map:
    return this->map_;
  case type_t::Tagged:
    return this->array_.front().to_map();
  default:
    return std::map<DataItem, DataItem>();
  }
}
simple DataItem::to_simple() const {
  switch (this->type_) {
  case type_t::Tagged:
    return this->array_.front().to_simple();
  case type_t::Simple:
    return simple(this->value_);
  default:
    return simple::Undefined;
  }
}

double DataItem::to_float() const {
  switch (this->type_) {
  case type_t::Unsigned:
    return double(this->value_);
  case type_t::Negative:
    return ldexp(-1 - int64_t(this->value_ >> 32), 32) +
           (-1 - int64_t(this->value_ << 32 >> 32));
  case type_t::Tagged:
    return this->array_.front().to_float();
  case type_t::Float:
    return this->float_;
  default:
    return 0.0;
  }
}

DataItem::operator bool() const {
  switch (this->type_) {
  case type_t::Tagged:
    return (bool)this->array_.front();
  case type_t::Simple:
    return this->value_ == simple::True;
  default:
    return false;
  }
}
DataItem::operator uint8_t() const { return this->to_unsigned(); }
DataItem::operator uint16_t() const { return this->to_unsigned(); }
DataItem::operator uint32_t() const { return this->to_unsigned(); }
DataItem::operator uint64_t() const { return this->to_unsigned(); }
DataItem::operator int8_t() const { return this->to_signed(); }
DataItem::operator int16_t() const { return this->to_signed(); }
DataItem::operator int32_t() const { return this->to_signed(); }
DataItem::operator int64_t() const { return this->to_signed(); }
DataItem::operator float() const { return this->to_float(); }
DataItem::operator double() const { return this->to_float(); }

DataItem::operator std::vector<uint8_t>() const { return this->to_binary(); }
DataItem::operator std::string() const { return this->to_string(); }
DataItem::operator std::vector<DataItem>() const { return this->to_array(); }
DataItem::operator std::map<DataItem, DataItem>() const {
  return this->to_map();
}
DataItem::operator cbor::simple() const { return this->to_simple(); }

DataItem &DataItem::operator[](const DataItem &key) {
  type_ = type_t::Map; // TODO type is null?
  return map_[key];
}

DataItem &DataItem::operator[](const DataItem &&key) {
  type_ = type_t::Map;
  return map_[key];
}

DataItem &DataItem::operator[](const char *key) {
  type_ = type_t::Map;
  return map_[key];
}

void DataItem::operator=(const std::string &str) {
  this->string_ = str;
  this->type_ = type_t::String;
}

void DataItem::operator=(const char *str) {
  this->string_ = str;
  this->type_ = type_t::String;
}

bool DataItem::operator<(const DataItem &other) const {
  if (this->type_ < other.type_) {
    return true;
  }
  if (this->type_ > other.type_) {
    return false;
  }
  switch (this->type_) {
  case type_t::Binary:
    return this->binary_ < other.binary_;
  case type_t::String:
    return this->string_ < other.string_;
  case type_t::Array:
    return this->array_ < other.array_;
  case type_t::Map:
    return this->map_ < other.map_;
  case type_t::Tagged:
    if (this->value_ < other.value_) {
      return true;
    }
    if (this->value_ > other.value_) {
      return false;
    }
    return this->array_.front() < other.array_.front();
  default:
    return this->value_ < other.value_;
  }
}
bool DataItem::operator==(const DataItem &other) const {
  if (this->type_ != other.type_) {
    return false;
  }
  switch (this->type_) {
  case type_t::Binary:
    return this->binary_ == other.binary_;
  case type_t::String:
    return this->string_ == other.string_;
  case type_t::Array:
    return this->array_ == other.array_;
  case type_t::Map:
    return this->map_ == other.map_;
  case type_t::Tagged:
    if (this->value_ != other.value_) {
      return false;
    }
    return this->array_ == other.array_;
  default:
    return this->value_ == other.value_;
  }
}
bool DataItem::operator!=(const DataItem &other) const {
  return !(*this == other);
}

/* ----------------------- decoder ----------------------- */
void read_uint(std::istream &in, int &major, int &minor, uint64_t &value) {
  major = (in.peek() >> 5) & 7;
  minor = in.get() & 31;
  value = 0;
  switch (minor) {
  case 27:
    value |= (uint64_t)in.get() << 56;
    value |= (uint64_t)in.get() << 48;
    value |= (uint64_t)in.get() << 40;
    value |= (uint64_t)in.get() << 32;
  case 26:
    value |= in.get() << 24;
    value |= in.get() << 16;
  case 25:
    value |= in.get() << 8;
  case 24:
    value |= in.get();
    break;
  default:
    value = minor;
    break;
  }
}

/* ----------------------- encoder ----------------------- */
void write_uint8(std::ostream &out, int major, uint64_t value) {
  if (value < 24) {
    out.put(major << 5 | value); // simple;
  } else {
    out.put(major << 5 | 24);
    out.put(value);
  }
}
void write_uint16(std::ostream &out, int major, uint64_t value) {
  out.put(major << 5 | 25);
  out.put(value >> 8);
  out.put(value);
}
void write_uint32(std::ostream &out, int major, uint64_t value) {
  out.put(major << 5 | 26);
  out.put(value >> 24);
  out.put(value >> 16);
  out.put(value >> 8);
  out.put(value);
}
void write_uint64(std::ostream &out, int major, uint64_t value) {
  out.put(major << 5 | 27);
  out.put(value >> 56);
  out.put(value >> 48);
  out.put(value >> 40);
  out.put(value >> 32);
  out.put(value >> 24);
  out.put(value >> 16);
  out.put(value >> 8);
  out.put(value);
}
void write_uint(std::ostream &out, int major, uint64_t value) {
  if ((value >> 8) == 0) {
    write_uint8(out, major, value);
  } else if ((value >> 16) == 0) {
    write_uint16(out, major, value);
  } else if (value >> 32 == 0) {
    write_uint32(out, major, value);
  } else {
    write_uint64(out, major, value);
  }
}
void write_float(std::ostream &out, double value) {
  if (double(float(value)) == value) {
    union {
      float f;
      uint32_t i;
    };
    f = value;
    write_uint32(out, 7, i);
  } else {
    union {
      double f;
      uint64_t i;
    };
    f = value;
    write_uint64(out, 7, i);
  }
}

void DataItem::set_os_mode(stream_mode mode) { output_mode_ = mode; }

bool DataItem::validate(const std::vector<uint8_t> &in) {
  std::istringstream buf1(std::string(in.begin(), in.end()));
  DataItem buf2;
  return buf2.read(buf1) && buf1.peek() == EOF;
}

bool DataItem::read(std::istream &in) {
  DataItem item;
  int major = 0;
  int minor = 0;
  uint64_t value = 0;
  read_uint(in, major, minor, value);
  switch (major) {
  case major::Unsigned:
    if (minor > 27) {
      in.setstate(std::ios_base::failbit);
      return false;
    }
    item.type_ = type_t::Unsigned;
    item.value_ = value;
    break;
  case major::Negative:
    if (minor > 27) {
      in.setstate(std::ios_base::failbit);
      return false;
    }
    item.type_ = type_t::Negative;
    item.value_ = value;
    break;
  case major::ByteString:
    if (minor > 27 && minor < 31) {
      in.setstate(std::ios_base::failbit);
      return false;
    }
    item.type_ = type_t::Binary;
    if (minor == 31) {
      while (in.good() && in.peek() != 255) {
        read_uint(in, major, minor, value);
        if (major != 2 || minor > 27) {
          in.setstate(std::ios_base::failbit);
          return false;
        }
        for (uint64_t i = 0; in.good() && i != value; ++i) {
          item.binary_.push_back(in.get());
        }
      }
      in.get();
    } else {
      for (uint64_t i = 0; in.good() && i != value; ++i) {
        item.binary_.push_back(in.get());
      }
    }
    break;
  case major::TextString:
    if (minor > 27 && minor < 31) {
      in.setstate(std::ios_base::failbit);
      return false;
    }
    item.type_ = type_t::String;
    if (minor == 31) {
      while (in.good() && in.peek() != 255) {
        read_uint(in, major, minor, value);
        if (major != 3 || minor > 27) {
          in.setstate(std::ios_base::failbit);
          return false;
        }
        for (uint64_t i = 0; in.good() && i != value; ++i) {
          item.string_.push_back(in.get());
        }
      }
      in.get();
    } else {
      for (uint64_t i = 0; in.good() && i != value; ++i) {
        item.string_.push_back(in.get());
      }
    }
    break;
  case major::Array:
    if (minor > 27 && minor < 31) {
      in.setstate(std::ios_base::failbit);
      return false;
    }
    item.type_ = type_t::Array;
    if (minor == 31) {
      while (in.good() && in.peek() != 255) {
        DataItem child;
        child.read(in);
        item.array_.push_back(child);
      }
      in.get();
    } else {
      for (uint64_t i = 0; in.good() && i != value; ++i) {
        DataItem child;
        child.read(in);
        item.array_.push_back(child);
      }
    }
    break;
  case major::Map:
    if (minor > 27 && minor < 31) {
      in.setstate(std::ios_base::failbit);
      return false;
    }
    item.type_ = type_t::Map;
    if (minor == 31) {
      while (in.good() && in.peek() != 255) {
        DataItem key, value;
        key.read(in);
        value.read(in);
        item.map_.insert(std::make_pair(key, value));
      }
      in.get();
    } else {
      for (uint64_t i = 0; in.good() && i != value; ++i) {
        DataItem key, value;
        key.read(in);
        value.read(in);
        item.map_.insert(std::make_pair(key, value));
      }
    }
    break;
  case major::Tag: {
    if (minor > 27) {
      in.setstate(std::ios_base::failbit);
      return false;
    }
    item.type_ = type_t::Tagged;
    item.value_ = value;
    DataItem child;
    child.read(in);
    item.array_.push_back(child);
    break;
  }
  case major::Simple:
    if (minor > 27) {
      in.setstate(std::ios_base::failbit);
      return false;
    }
    switch (minor) {
    case 25: {
      item.type_ = type_t::Float;
      int sign = value >> 15;
      int exponent = value >> 10 & 31;
      int significand = value & 1023;
      if (exponent == 31) {
        if (significand) {
          item.float_ = NAN;
        } else if (sign) {
          item.float_ = -INFINITY;
        } else {
          item.float_ = INFINITY;
        }
      } else if (exponent == 0) {
        if (sign) {
          item.float_ = -ldexp(significand, -24);
        } else {
          item.float_ = ldexp(significand, -24);
        }
      } else {
        if (sign) {
          item.float_ = -ldexp(1024 | significand, exponent - 25);
        } else {
          item.float_ = ldexp(1024 | significand, exponent - 25);
        }
      }
      break;
    }
    case 26: {
      union {
        float f;
        uint32_t i;
      };
      i = value;
      item.type_ = type_t::Float;
      item.float_ = f;
      break;
    }
    case 27:
      item.type_ = type_t::Float;
      item.value_ = value;
      break;
    default:
      item.type_ = type_t::Simple;
      item.value_ = value;
    }
    break;
  }
  if (!in.good()) {
    in.setstate(std::ios_base::failbit);
    return false;
  }
  *this = item;
  return true;
}

void DataItem::write(std::ostream &out) const {
  switch (this->type_) {
  case type_t::Unsigned:
    write_uint(out, 0, this->value_);
    break;
  case type_t::Negative:
    write_uint(out, 1, this->value_);
    break;
  case type_t::Binary:
    write_uint(out, 2, this->binary_.size());
    out.write(reinterpret_cast<const char *>(this->binary_.data()),
              this->binary_.size());
    break;
  case type_t::String:
    write_uint(out, 3, this->string_.size());
    out.write(this->string_.data(), this->string_.size());
    break;
  case type_t::Array:
    write_uint(out, 4, this->array_.size());
    for (std::vector<DataItem>::const_iterator it = this->array_.begin();
         it != this->array_.end(); ++it) {
      it->write(out);
    }
    break;
  case type_t::Map:
    write_uint(out, 5, this->map_.size());
    for (std::map<DataItem, DataItem>::const_iterator it = this->map_.begin();
         it != this->map_.end(); ++it) {
      it->first.write(out);
      it->second.write(out);
    }
    break;
  case type_t::Tagged:
    write_uint(out, 6, this->value_);
    this->array_.front().write(out);
    break;
  case type_t::Simple:
    write_uint8(out, 7, this->value_);
    break;
  case type_t::Float:
    write_float(out, this->float_);
    break;
  }
}

// TODO honor the indentation;
std::string DataItem::dump(int indent) const {
  std::ostringstream out;
  switch (type_) {
  case type_t::Unsigned:
    out << value_;
    break;
  case type_t::Negative:
    if (1 + value_ == 0) {
      out << "-18446744073709551616";
    } else {
      out << "-" << 1 + value_;
    }
    break;
  case type_t::Binary:
    out << "h'";
    out << std::hex;
    out.fill('0');
    for (std::vector<uint8_t>::const_iterator it = binary_.begin();
         it != binary_.end(); ++it) {
      out.width(2);
      out << int(*it);
    }
    out << "'";
    break;
  case type_t::String:
    out << "\"";
    out << std::hex;
    out.fill('0');
    for (std::string::const_iterator it = string_.begin(); it != string_.end();
         ++it) {
      switch (*it) {
      case '\n':
        out << "\\n";
        break;
      case '\r':
        out << "\\r";
        break;
      case '\"':
        out << "\\\"";
        break;
      case '\\':
        out << "\\\\";
        break;
      default:
        if ((unsigned char)*it < '\x20') {
          out << "\\u";
          out.width(4);
          out << (int)(unsigned char)*it;
        } else {
          out << *it;
        }
        break;
      }
    }
    out << "\"";
    break;
  case type_t::Array:
    out << "[";
    for (std::vector<DataItem>::const_iterator it = array_.begin();
         it != array_.end(); ++it) {
      if (it != array_.begin()) {
        out << ", ";
      }
      out << it->dump(indent);
    }
    out << "]";
    break;
  case type_t::Map:
    out << "{";
    for (std::map<DataItem, DataItem>::const_iterator it = map_.begin();
         it != map_.end(); ++it) {
      if (it != map_.begin()) {
        out << ", ";
      }
      out << it->first.dump(indent) << ": " << it->second.dump(indent);
    }
    out << "}";
    break;
  case type_t::Tagged:
    out << value_ << "(" << array_.front().dump(indent) << ")";
    break;
  case type_t::Simple:
    switch (value_) {
    case simple::False:
      out << "false";
      break;
    case simple::True:
      out << "true";
      break;
    case simple::Null:
      out << "null";
      break;
    case simple::Undefined:
      out << "undefined";
      break;
    default:
      out << "simple(" << value_ << ")"; // TODO
      break;
    }
    break;
  case type_t::Float:
    if (std::isinf(float_)) {
      if (float_ < 0) {
        out << "-";
      }
      out << "Infinity";
    } else if (std::isnan(float_)) {
      out << "NaN";
    } else {
      out << std::showpoint << float_;
    }
    break;
  }
  return out.str();
}

} // namespace cbor