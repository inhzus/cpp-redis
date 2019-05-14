//
// Created by suun on 5/13/19.
//

#ifndef REDIS_DICT_H
#define REDIS_DICT_H
#include <utility>
#include <hash_map>
#include "sds.h"

namespace std {
template<>
struct hash<rd::String> {
  std::size_t operator()(const rd::String &key) const;
};
}

namespace {

using std::hash;

template<class K, class V>
class _DictIterator;

template<class K, class V>
class Dict;

template<class K, class V>
struct _DictEntry {
  struct { K key; V value; } data;
  _DictEntry *next;
  _DictEntry(const K &k, const V &v);

};

template<class K, class V>
struct _DictTable {
  _DictEntry<K, V> **table_;
  size_t capacity_;
  size_t size_;

  size_t capacityMask() const { return capacity_ - 1; }

  _DictTable();
  _DictTable(size_t n);
  ~_DictTable();
};

template<class K, class V>
class _DictIterator {
 public:
  typedef _DictIterator<K, V> iterator;
  typedef std::forward_iterator_tag iterator_category;
  typedef _DictEntry<K, V> entry_type;
  typedef Dict<K, V> dict_type;
  typedef entry_type &reference;
  typedef entry_type *pointer;

 private:
  entry_type *cur_;
  dict_type *dict_;
  bool in_rehash_;
 public:
  _DictIterator()
      : cur_(nullptr), dict_(nullptr), in_rehash_(false) {}
  _DictIterator(entry_type *cur, dict_type *dict);
  ~_DictIterator();

  reference operator*()
  const { return cur_->data; }

  reference operator->()
  const { return &(operator*()); }

  iterator &operator++();
  iterator operator++(int);

  bool operator==(const iterator &it)
  const { return cur_ == it.cur_; }
  bool operator!=(const iterator &it)
  const { return !(operator==(it)); }
};

template<class K, class V>
class Dict {
 public:
  typedef K key_type;
  typedef V value_type;
  typedef _DictEntry<K, V> entry_type;

  typedef size_t size_type;
  typedef entry_type *pointer;
  typedef const entry_type *const_pointer;
  typedef entry_type &reference;
  typedef const entry_type &const_reference;

  typedef _DictIterator<K, V> iterator;

 private:
  static const size_type kInitialSize = 4;

  _DictTable<K, V> table_;
  _DictTable<K, V> rehash_;
  size_type process_;

  void rehash(size_type n);

 public:
  Dict();
  ~Dict();

  iterator get(const K &key);
  iterator add(const K &key, const V &value);
  iterator replace(const K &key, const V &value);
  bool remove(const K &key);
  iterator find(const K &key);
  template<class UnFn>
  size_type scan(size_type n, UnFn fn);

  void _acquireIterator();
  void _releaseIterator();
};

}
#endif //REDIS_DICT_H
