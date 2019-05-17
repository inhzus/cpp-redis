//
// Created by suun on 5/13/19.
//

#ifndef REDIS_DICT_H
#define REDIS_DICT_H
// vector used in _DictTable.table_
#include <vector>
#include "common.h"

namespace rd {

using std::hash;
using std::vector;

class _DictIterator;

class Dict;

struct _DictEntry {
  Var key;
  Var value;
  _DictEntry *next;
  _DictEntry(Var &k, Var &v);
};

struct _DictTable {
  vector<_DictEntry *> table_;
  size_type capacity_;
  size_type size_;
  Allocator<_DictEntry> allocator_;

  inline size_t mask() const { return capacity_ - 1; }

//  _DictTable();
  explicit _DictTable(size_t n);
  ~_DictTable();
  _DictEntry *
  &at(size_type n) { return table_[n]; }
};

class _DictIterator {
  friend class Dict;

 public:
  typedef rd::size_type size_type;
  typedef _DictIterator iterator;
  typedef std::forward_iterator_tag iterator_category;
  typedef _DictEntry entry_type;
  typedef Dict dict_type;
  typedef entry_type &reference;
  typedef entry_type *pointer;

 private:
  pointer cur_;
  Dict *dict_;
  bool in_rehash_;
  size_type index_;

 public:
  _DictIterator()
      : cur_(nullptr), dict_(nullptr),
        index_(0), in_rehash_(false) {}
  _DictIterator(const _DictIterator &it);
  explicit _DictIterator(
      Dict *dict, size_type index = 0,
      pointer cur = nullptr, bool in_rehash = false);
  ~_DictIterator();

  reference operator*()
  const { return *cur_; }

  pointer operator->()
  const { return cur_; }

  iterator &operator++();
  iterator operator++(int); // NOLINT

  bool operator==(const iterator &it)
  const { return dict_ == it.dict_ && cur_ == it.cur_; }
  bool operator!=(const iterator &it)
  const { return !(operator==(it)); }
  bool operator==(nullptr_t)
  const { return cur_ == nullptr; }
  bool operator!=(nullptr_t)
  const { return cur_ != nullptr; }
};

class Dict {
 public:
  friend class _DictIterator;

  typedef Var key_type;
  typedef Var value_type;
  typedef rd::size_type size_type;
  typedef _DictIterator iterator;
  typedef _DictEntry entry_type;
  typedef entry_type *pointer;
  typedef entry_type &reference;

 private:
  const size_type kForceResizeRatio = 5;
  const size_type kInitialSize = 4;
  const size_type kRehashSliceLength = 10;
  const size_type kRehashMsDuration = 100;
  const size_type kResizeRatio = 1;

  _DictTable *data_, *rehash_;
  size_type process_;
  size_type iter_num_;
  bool resizable;
  Allocator<_DictTable> allocator_;

  size_type fixSize(size_type n) const;
  template<class T>
  T reverseBit(T n) const;

  iterator findIterIndex(const key_type &key);
  typename Dict::iterator setKeyValue(
      const iterator &iter, key_type &key, value_type &value);
  void rehash(size_type n = 1);
  void resize(size_type n);

  inline void stopRehash();
  inline void acquireIterator(iterator *iter);
  inline void releaseIterator();

 public:
  Dict();
  ~Dict();

  iterator begin();
  iterator end();
  size_type size() const;
  bool empty() const;

  bool isRehashing()
  const { return rehash_ != nullptr; }
  bool isRehashable()
  const { return iter_num_ == 0; }

  void rehashMilliseconds(size_type n);
  void shrink();
  void expand();

  iterator get(const key_type &key);
  iterator add(key_type &key, value_type &value);
  iterator replace(key_type &key, value_type &value);
  bool remove(const key_type &key);
  template<class UnFn>
  size_type scan(size_type n, UnFn fn);

};

template<class T>
T Dict::reverseBit(T n) const {
  T s = 8 * sizeof(n),
      mask = ~T();
  while ((s >>= 1u) > 0) {
    mask ^= (mask << s);
    n = ((n >> s) & mask) | ((n << s) & ~mask);
  }
  return n;
}

template<class UnFn>
size_type Dict::scan(size_type n, UnFn fn) {
  if (empty()) { return 0; }
  if (isRehashing()) {
    _DictTable *foo = data_, *bar = rehash_;
    if (foo->size_ > bar->size_) {
      foo = rehash_;
      bar = data_;
    }
    size_type foo_mask = foo->mask(), bar_mask = bar->mask();
    for (pointer entry = foo->at(n & foo_mask);
         entry != nullptr; entry = entry->next) {
      fn(entry);
    }
    do {
      for (pointer entry = bar->at(n & bar_mask);
           entry != nullptr; entry = entry->next) {
        fn(entry);
      }
      n |= ~bar_mask;
      n = reverseBit(n);
      n++;
      n = reverseBit(n);
    } while (n & (foo_mask ^ bar_mask));
  } else {
    _DictTable *table = data_;
    size_type mask = table->mask();
    for (pointer entry = table->at(n & mask);
         entry != nullptr; entry = entry->next) {
      fn(entry);
    }
    n |= ~mask;
    n = reverseBit(n);
    n++;
    n = reverseBit(n);
  }
  return n;
}
}

#endif //REDIS_DICT_H
