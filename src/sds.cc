//
// Created by suun on 5/13/19.
//

#include "sds.h"
namespace rd {

String::size_type String::getNewCapacity(String::size_type n) const {
  size_type _old = storage_end_ - begin_;
  return _old + (_old > n ? _old : n);
}

String::String(const char *s) {
  allocateAndCopy(s, s + strlen(s));
}
String::String(const String &string) {
  allocateAndCopy(string.begin_, string.end_);
}
String::String(const char *s, String::size_type n) {
  constructAux(s, n, typename std::__is_integer<size_type>::__type());
}
String::~String() {
  if (begin_ != nullptr) {
//    allocator_.deallocate(begin_, capacity() + 1);
    delete[] begin_;
    storage_end_ = end_ = begin_ = nullptr;
  }
}
void String::clear() {
  end_ = begin_;
  *end_ = 0;
}
const char *String::data() const {
  return begin_;
}

String &String::assign(const String &string) {
  return assignAux(string.begin(), string.end(), std::__false_type());
}
String &String::assign(const char *s) {
  return assignAux(s, s + strlen(s), std::__false_type());
}
String &String::assign(const char *s, String::size_type n) {
  return assignAux(s, n, std::__is_integer<size_type>::__type());
}
void String::resize(String::size_type n) {
  if (n < size()) {
    std::uninitialized_fill(begin_ + n, end_, 0);
  } else if (n < capacity()) {
    end_ = std::uninitialized_fill_n(end_, n, 0);
  } else {
    size_type offset = n - size();
    size_type newCapacity = getNewCapacity(offset);
//    iterator newBegin = allocator_.allocate(newCapacity + 1);
    auto newBegin = new value_type[newCapacity + 1];
    end_ = std::uninitialized_copy(begin_, end_, newBegin);
    end_ = std::uninitialized_fill_n(end_, offset, 0);
//    allocator_.deallocate(begin_, capacity() + 1);
    delete[] begin_;
    begin_ = newBegin;
    storage_end_ = begin_ + newCapacity;
  }
  *end_ = 0;
}
String String::substr(String::size_type pos, String::size_type offset) const {
  return String(begin_ + pos, begin_ + pos + offset);
}
void String::trim(const char *set) {
  iterator newBegin, newEnd;
  for (newBegin = begin_; newBegin != end_ && strchr(set, *newBegin); newBegin++);
  for (newEnd = end_ - 1; newEnd != begin_ - 1 && strchr(set, *newEnd); newEnd--);
  newEnd++;
  std::uninitialized_copy(newBegin, newEnd, begin_);
  end_ = begin_ + (newEnd - newBegin);
  *end_ = 0;
}
String &String::operator=(const String &string) {
  return assign(string); // NOLINT(cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator)
}
String &String::operator=(const char *s) {
  return assign(s); // NOLINT(cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator)
}
String &String::operator+=(const char *s) {
  return append(s, s + strlen(s));
}
String &String::operator+=(const String &string) {
  return append(string.begin_, string.end_);
}
String operator+(const String &lhs, const String &rhs) {
  String ret(lhs);
  return ret += rhs;
}
String operator+(const String &lhs, const char *rhs) {
  String ret(lhs);
  return ret += rhs;
}
String operator+(const char *lhs, const String &rhs) {
  String ret(lhs);
  return ret += rhs;
}
bool operator==(const rd::String &lhs, const rd::String &rhs) {
  rd::String::iterator lit = lhs.begin(), rit = rhs.begin();
  for (; lit != lhs.end() && rit != rhs.end() && *lit == *rit; lit++, rit++);
  return (lit == lhs.end() && rit == rhs.end());
}
bool operator!=(const rd::String &lhs, const rd::String &rhs) {
  return !(lhs == rhs);
}
std::ostream &operator<<(std::ostream &os, const rd::String &string) {
  return os << string.begin_;
}
}
