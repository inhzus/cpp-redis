//
// Created by suun on 9/25/2018.
//

#ifndef REDIS_SDS_H
#define REDIS_SDS_H

#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
namespace rd {
class String {
 public:
  typedef char value_type;
  typedef char *iterator;
  typedef const char *const_iterator;
  typedef size_t size_type;
  typedef char &reference;
  typedef const char &const_reference;
  typedef ptrdiff_t difference_type;
 private:
  iterator _begin{};
  iterator _end{};
  iterator _storage_end{};

  std::allocator<value_type> allocator;

  template<class InIt>
  void allocateAndCopy(InIt first, InIt last);
  size_type getNewCapacity(size_type n) const;
  template<class InIt>
  void construct_aux(InIt first, InIt last, std::__false_type);
  template<class InIt>
  void construct_aux(InIt ite, size_type n, std::__true_type);
  template<class InIt>
  String &assign_aux(InIt first, InIt last, std::__false_type);
  template<class InIt>
  String &assign_aux(InIt ite, size_type n, std::__true_type);

 public:
  String() : _begin(nullptr), _end(nullptr), _storage_end(nullptr) {}
  explicit String(const char *s);
  String(const String &string);
  template<class InIt>
  String(InIt first, InIt last);
  String(const char *s, size_type n);
  ~String();

  iterator begin() const { return _begin; }
  iterator end() const { return _end; }
  size_type capacity() const { return _storage_end - _begin; }
  size_type size() const { return _end - _begin; }
  size_type empty() const { return _end == _begin; }
  void clear();

  template<class InIt>
  String &append(InIt first, InIt last);
  String &assign(const String &string);
  String &assign(const char *s);
  String &assign(const char *s, size_type n);
  template<class InIt>
  String &assign(InIt first, InIt last);
  void resize(size_type n);
  String substr(size_type pos, size_type offset) const;
  void trim(const char *set);

  String &operator=(const String &string);
  String &operator=(const char *s);
  String &operator+=(const char *s);
  String &operator+=(const String &string);
  friend String operator+(const String &lhs, const String &rhs);
  friend String operator+(const String &lhs, const char *rhs);
  friend String operator+(const char *lhs, const String &rhs);
  friend bool operator==(const rd::String &lhs, const rd::String &rhs);
  friend bool operator!=(const rd::String &lhs, const rd::String &rhs);
  friend std::ostream &operator<<(std::ostream &os, const rd::String &string);
};

template<class InIt>
void String::allocateAndCopy(InIt first, InIt last) {
  auto len = static_cast<size_type>(std::distance(first, last));
  _begin = allocator.allocate(len + 1);
  std::uninitialized_copy(first, last, _begin);
  _storage_end = _end = _begin + len;
  *_end = 0;
}
template<class InIt>
void String::construct_aux(InIt first, InIt last, std::__false_type) {
  allocateAndCopy(first, last);
}
template<class InIt>
void String::construct_aux(InIt ite, String::size_type n, std::__true_type) {
  allocateAndCopy(ite, ite + n);
}
template<class InIt>
String::String(InIt first, InIt last) {
  construct_aux(first, last, typename std::__is_integer<InIt>::__type());
}
template<class InIt>
String &String::append(InIt first, InIt last) {
  auto offset = static_cast<size_type>(std::distance(first, last));
  if (offset < _storage_end - _end) {
    while (first != last)
      *_end++ = *first++;
    *_end = 0;
  } else {
    size_type newCapacity = getNewCapacity(offset);
    iterator newBeginIterator = allocator.allocate(newCapacity + 1);
    _end = std::uninitialized_copy(_begin, _end, newBeginIterator);
    _end = std::uninitialized_copy(first, last, _end);
    allocator.deallocate(_begin, capacity() + 1);
    _begin = newBeginIterator;
    *_end = 0;
    _storage_end = newBeginIterator + newCapacity;
  }
  return *this;
}
template<class InIt>
String &String::assign_aux(InIt first, InIt last, std::__false_type) {
  auto len = static_cast<size_type>(std::distance(first, last));
  if (len < capacity()) {
    std::uninitialized_copy(first, last, _begin);
    _end = _begin + len;
    *_end = 0;
  } else {
    allocator.deallocate(_begin, capacity());
    _begin = allocator.allocate(len + 1);
    _storage_end = _end = std::uninitialized_copy(first, last, _begin);
    *_end = 0;
  }
  return *this;
}
template<class InIt>
String &String::assign_aux(InIt ite, String::size_type n, std::__true_type) {
  return assign_aux(ite, ite + n, std::__false_type());
}
template<class InIt>
String &String::assign(InIt first, InIt last) {
  return assign_aux(first, last, std::__is_integer<InIt>::__type());
}

}

#endif //REDIS_SDS_H
