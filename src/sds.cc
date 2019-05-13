//
// Created by suun on 5/13/19.
//

#include "sds.h"
namespace rd {

String::size_type String::getNewCapacity(String::size_type n) const {
  size_type _old = _storage_end - _begin;
  return _old + (_old > n ? _old : n);
}

String::String(const char *s) {
  allocateAndCopy(s, s + strlen(s));
}
String::String(const String &string) {
  allocateAndCopy(string._begin, string._end);
}
String::String(const char *s, String::size_type n) {
  construct_aux(s, n, typename std::__is_integer<size_type>::__type());
}
String::~String() {
  if (_begin != nullptr) {
    _storage_end = _end = _begin = nullptr;
    allocator.deallocate(_begin, capacity() + 1);
  }
}
void String::clear() {
  _end = _begin;
  *_end = 0;
}

String &String::assign(const String &string) {
  return assign_aux(string.begin(), string.end(), std::__false_type());
}
String &String::assign(const char *s) {
  return assign_aux(s, s + strlen(s), std::__false_type());
}
String &String::assign(const char *s, String::size_type n) {
  return assign_aux(s, n, std::__is_integer<size_type>::__type());
}
void String::resize(String::size_type n) {
  if (n < size()) {
    std::uninitialized_fill(_begin + n, _end, 0);
  } else if (n < capacity()) {
    _end = std::uninitialized_fill_n(_end, n, 0);
  } else {
    size_type offset = n - size();
    size_type newCapacity = getNewCapacity(offset);
    iterator newBegin = allocator.allocate(newCapacity + 1);
    _end = std::uninitialized_copy(_begin, _end, newBegin);
    _end = std::uninitialized_fill_n(_end, offset, 0);
    allocator.deallocate(_begin, capacity() + 1);
    _begin = newBegin;
    _storage_end = _begin + newCapacity;
  }
  *_end = 0;
}
String String::substr(String::size_type pos, String::size_type offset) const {
  return String(_begin + pos, _begin + pos + offset);
}
void String::trim(const char *set) {
  iterator newBegin, newEnd;
  for (newBegin = _begin; newBegin != _end && strchr(set, *newBegin); newBegin++);
  for (newEnd = _end - 1; newEnd != _begin - 1 && strchr(set, *newEnd); newEnd--);
  newEnd++;
  std::uninitialized_copy(newBegin, newEnd, _begin);
  _end = _begin + (newEnd - newBegin);
  *_end = 0;
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
  return append(string._begin, string._end);
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
  return os << string._begin;
}
}
