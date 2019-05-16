//
// Created by suun on 9/25/2018.
//

#ifndef REDIS_SDS_H
#define REDIS_SDS_H

// For strlen, strchr
#include <cstring>
// For partial specialization std::hash
#include <functional>
// For ostream
#include <iostream>
#include "common.h"

namespace rd {
class String {
 public:
  typedef char value_type;
  typedef char *iterator;
  typedef const char *const_iterator;
  typedef rd::size_type size_type;
  typedef char &reference;
  typedef const char &const_reference;
  typedef ptrdiff_t difference_type;

 private:
  iterator begin_{};
  iterator end_{};
  iterator storage_end_{};

  Allocator<value_type> allocator_;

  template<class InIt>
  void allocateAndCopy(InIt first, InIt last);
  size_type getNewCapacity(size_type n) const;
  template<class InIt>
  void constructAux(InIt first, InIt last, std::__false_type);
  template<class InIt>
  void constructAux(InIt ite, size_type n, std::__true_type);
  template<class InIt>
  String &assignAux(InIt first, InIt last, std::__false_type);
  template<class InIt>
  String &assignAux(InIt ite, size_type n, std::__true_type);

 public:
  String() : begin_(nullptr), end_(nullptr), storage_end_(nullptr) {}
  explicit String(const char *s);
  String(const String &string);
  template<class InIt>
  String(InIt first, InIt last);
  String(const char *s, size_type n);
  ~String();

  iterator begin() const { return begin_; }
  iterator end() const { return end_; }
  size_type capacity() const { return storage_end_ - begin_; }
  size_type size() const { return end_ - begin_; }
  size_type empty() const { return end_ == begin_; }
  void clear();
  const char *data() const;

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
  begin_ = allocator_.allocate(len + 1);
  std::uninitialized_copy(first, last, begin_);
  storage_end_ = end_ = begin_ + len;
  *end_ = 0;
}
template<class InIt>
void String::constructAux(InIt first, InIt last, std::__false_type) {
  allocateAndCopy(first, last);
}
template<class InIt>
void String::constructAux(InIt ite, String::size_type n, std::__true_type) {
  allocateAndCopy(ite, ite + n);
}
template<class InIt>
String::String(InIt first, InIt last) {
  constructAux(first, last, typename std::__is_integer<InIt>::__type());
}
template<class InIt>
String &String::append(InIt first, InIt last) {
  auto offset = static_cast<size_type>(std::distance(first, last));
  if (offset < storage_end_ - end_) {
    while (first != last)
      *end_++ = *first++;
    *end_ = 0;
  } else {
    size_type newCapacity = getNewCapacity(offset);
    iterator newBeginIterator = allocator_.allocate(newCapacity + 1);
    end_ = std::uninitialized_copy(begin_, end_, newBeginIterator);
    end_ = std::uninitialized_copy(first, last, end_);
    allocator_.deallocate(begin_, capacity() + 1);
    begin_ = newBeginIterator;
    *end_ = 0;
    storage_end_ = newBeginIterator + newCapacity;
  }
  return *this;
}
template<class InIt>
String &String::assignAux(InIt first, InIt last, std::__false_type) {
  auto len = static_cast<size_type>(std::distance(first, last));
  if (len < capacity()) {
    std::uninitialized_copy(first, last, begin_);
    end_ = begin_ + len;
    *end_ = 0;
  } else {
    allocator_.deallocate(begin_, capacity());
    begin_ = allocator_.allocate(len + 1);
    storage_end_ = end_ = std::uninitialized_copy(first, last, begin_);
    *end_ = 0;
  }
  return *this;
}
template<class InIt>
String &String::assignAux(InIt ite, String::size_type n, std::__true_type) {
  return assignAux(ite, ite + n, std::__false_type());
}
template<class InIt>
String &String::assign(InIt first, InIt last) {
  return assignAux(first, last, std::__is_integer<InIt>::__type());
}

}

namespace std {
template<>
struct hash<rd::String> {
  std::size_t operator()(const rd::String &key) const {
    return std::_Hash_impl::hash(key.data(), key.size());
  }
};
}

#endif //REDIS_SDS_H
