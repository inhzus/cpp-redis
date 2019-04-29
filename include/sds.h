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
  void allocateAndCopy(InIt first, InIt last) {
    auto len = static_cast<size_type>(std::distance(first, last));
    _begin = allocator.allocate(len + 1);
    std::uninitialized_copy(first, last, _begin);
    _storage_end = _end = _begin + len;
    *_end = 0;
  }
/*

  void allocateAndFill(size_type n, value_type ch) {
    _begin = allocator.allocate(n + 1);
    std::uninitialized_fill_n(_begin, n, ch);
    _storage_end = _end = _begin + n;
    *_end = 0;
  }
*/

  size_type getNewCapacity(size_type n) const {
    size_type _old = _storage_end - _begin;
    return _old + (_old > n ? _old : n);
  }

  template<class InIt>
  void construct_aux(InIt first, InIt last, std::__false_type) {
    allocateAndCopy(first, last);
  }

  template<class InIt>
  void construct_aux(InIt ite, size_type n, std::__true_type) {
    allocateAndCopy(ite, ite + n);
  }

 public:
  String() : _begin(nullptr), _end(nullptr), _storage_end(nullptr) {}
  explicit String(const char *s) {
    allocateAndCopy(s, s + strlen(s));
  }
  String(const String &string) {
    allocateAndCopy(string._begin, string._end);
  }
  template<class InIt>
  String(InIt first, InIt last) {
    construct_aux(first, last, typename std::__is_integer<InIt>::__type());
  }
  String(const char *s, size_type n) {
    construct_aux(s, n, typename std::__is_integer<size_type>::__type());
  }
  ~String() {
    if (_begin!=nullptr) {
      _storage_end = _end = _begin = nullptr;
      allocator.deallocate(_begin, capacity() + 1);
    }
  }

  iterator begin() const {
    return _begin;
  }

  iterator end() const {
    return _end;
  }

  size_type capacity() const {
    return _storage_end - _begin;
  }

  size_type size() const {
    return _end - _begin;
  }

  void clear() {
    _end = _begin;
    *_end = 0;
  }

  template<class InIt>
  String &append(InIt first, InIt last) {
    auto offset = static_cast<size_type>(std::distance(first, last));
    if (offset < _storage_end - _end) {
      while (first!=last)
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
  String &assign_aux(InIt first, InIt last, std::__false_type) {
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
  String &assign_aux(InIt ite, size_type n, std::__true_type) {
    return assign_aux(ite, ite + n, std::__false_type());
  }

  String &assign(const String &string) {
    return assign_aux(string.begin(), string.end(), std::__false_type());
  }

  String &assign(const char *s) {
    return assign_aux(s, s + strlen(s), std::__false_type());
  }

  String &assign(const char *s, size_type n) {
    return assign_aux(s, n, std::__is_integer<size_type>::__type());
  }

  template<class InIt>
  String &assign(InIt first, InIt last) {
    return assign_aux(first, last, std::__is_integer<InIt>::__type());
  }
 void resize(size_type n) {
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

  String substr(size_type pos, size_type offset) const {
    return String(_begin + pos, _begin + pos + offset);
  }

  void trim(const char *set) {
    iterator newBegin, newEnd;
    for (newBegin = _begin; newBegin!=_end && strchr(set, *newBegin); newBegin++);
    for (newEnd = _end - 1; newEnd!=_begin - 1 && strchr(set, *newEnd); newEnd--);
    newEnd++;
    std::uninitialized_copy(newBegin, newEnd, _begin);
    _end = _begin + (newEnd - newBegin);
    *_end = 0;
  }

  String &operator=(const String &string) {
    return assign(string); // NOLINT(cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator)
  }

  String &operator=(const char *s) {
    return assign(s); // NOLINT(cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator)
  }

  String &operator+=(const char *s) {
    return append(s, s + strlen(s));
  }

  String &operator+=(const String &string) {
    return append(string._begin, string._end);
  }

  friend String operator+(const String &lhs, const String &rhs) {
    String ret(lhs);
    return ret += rhs;
  }

  friend String operator+(const String &lhs, const char *rhs) {
    String ret(lhs);
    return ret += rhs;
  }

  friend String operator+(const char *lhs, const String &rhs) {
    String ret(lhs);
    const char *a = "SELECT DISTINCT cid, aid FROM Orders o NOT EXISTS (SELECT * FROM Agents a WHERE city='NEW WORK' AND a.cid=o.cid)";
    return ret += rhs;
  }

  friend bool operator==(const rd::String &lhs, const rd::String &rhs) {
    rd::String::iterator lit = lhs.begin(), rit = rhs.begin();
    for (; lit!=lhs.end() && rit!=rhs.end() && *lit==*rit; lit++, rit++);
    return (lit==lhs.end() && rit==rhs.end());
  }

  friend bool operator!=(const rd::String &lhs, const rd::String &rhs) {
    return !(lhs==rhs);
  }

  friend std::ostream &operator<<(std::ostream &os, const rd::String &string) {
    return os << string._begin;
  }
};
}


/*sds sds_new(const char *string);!
sds sds_empty(const char *string);!
sds sds_free(sds s);!
unsigned sds_len(const sds s);!
unsigned sds_avail(const sds s);!
sds sds_dump(const sds s);!
sds sds_cat(sds s, const char *string);!
sds sds_cat_sds(sds s);!
sds sds_copy(sds s, const char *string);!
sds sds_grow_zero(sds s, unsigned _len);!
sds sds_range(sds s, unsigned start, unsigned end);!
sds sds_trim(sds s, const char *string);!
int sds_comp(const sds s1, const sds s2);*/



#endif //REDIS_SDS_H
