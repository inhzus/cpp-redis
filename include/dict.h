//
// Created by suun on 5/13/19.
//

#ifndef REDIS_DICT_H
#define REDIS_DICT_H
// For assert
#include <cassert>
// For std::hash
#include <functional>
// Used in _DictTable.table_
#include <vector>
#include "common.h"

namespace rd {

using std::hash;
using std::vector;

struct Any {};

template<class T>
struct Value : public Any {
  T value;
  explicit Value(const T &val) : value(val) {}
  explicit Value(T &&val) : value(std::move(val)) {}
};

template<class K, class V>
class _DictIterator;

template<class K, class V>
class Dict;

template<class K, class V>
struct _DictEntry {
  K key;
  V value;
  _DictEntry *next;
  _DictEntry(K &k, V &v);
};

template<class K, class V>
struct _DictTable {
  vector<_DictEntry<K, V> *> table_;
  size_type capacity_;
  size_type size_;

  inline size_t mask() const { return capacity_ - 1; }

//  _DictTable();
  explicit _DictTable(size_t n);
  ~_DictTable();
  _DictEntry<K, V> *
  &at(size_type n) { return table_[n]; }
};

template<class K, class V>
class _DictIterator {
  friend class Dict<K, V>;

 public:
  typedef rd::size_type size_type;
  typedef _DictIterator<K, V> iterator;
  typedef std::forward_iterator_tag iterator_category;
  typedef _DictEntry<K, V> entry_type;
  typedef Dict<K, V> dict_type;
  typedef entry_type &reference;
  typedef entry_type *pointer;

 private:
  pointer cur_;
  Dict<K, V> *dict_;
  bool in_rehash_;
  size_type index_;

 public:
  _DictIterator()
      : cur_(nullptr), dict_(nullptr),
        index_(0), in_rehash_(false) {}
  _DictIterator(const _DictIterator &it);
  explicit _DictIterator(
      Dict<K, V> *dict, size_type index = 0,
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

template<class K, class V>
class Dict {
 public:
  friend class _DictIterator<K, V>;

  typedef K key_type;
  typedef V value_type;
  typedef rd::size_type size_type;
  typedef _DictIterator<K, V> iterator;
  typedef _DictEntry<K, V> entry_type;
  typedef entry_type *pointer;
  typedef entry_type &reference;

 private:
  const size_type kInitialSize = 4;
  const size_type kRehashSliceLength = 10;
  const size_type kResizeRatio = 1;
  const size_type kForceResizeRatio = 5;

  _DictTable<K, V> *data_, *rehash_;
  size_type process_;
  size_type iter_num_;
  bool resizable;

  size_type fixSize(size_type n) const;
  template<class T>
  T reverseBit(T n) const;

  iterator findIterIndex(const K &key);
  typename Dict<K, V>::iterator setKeyValue(
      _DictIterator<K, V> iter, K &key, V &value);
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

  void shrink();
  void expand();

  iterator get(const K &key);
  iterator add(K &key, V &value);
  iterator replace(const K &key, V &value);
  bool remove(const K &key);
  template<class UnFn>
  size_type scan(size_type n, UnFn fn);

};

template<class K, class V>
_DictEntry<K, V>::_DictEntry(K &k, V &v) : next() {
  key = std::move(k);
  value = std::move(v);
}

//template<class K, class V>
//_DictTable<K, V>::_DictTable()
//    : data_(), capacity_(0), size_(0) {}
template<class K, class V>
_DictTable<K, V>::_DictTable(size_t n)
    : capacity_(n), size_(0) {
  table_ = vector<_DictEntry<K, V> *>(n);
}
template<class K, class V>
_DictTable<K, V>::~_DictTable() {
  for (_DictEntry<K, V> *entry : table_) {
    while (entry != nullptr) {
      _DictEntry<K, V> *tmp = entry;
      entry = entry->next;
      delete (tmp);
    }
  }
}
template<class K, class V>
_DictIterator<K, V>::_DictIterator(const _DictIterator &it)
    : cur_(it.cur_), dict_(it.dict_),
      index_(it.index_), in_rehash_(it.in_rehash_) {
  dict_->acquireIterator(this);
}
template<class K, class V>
_DictIterator<K, V>::_DictIterator(
    Dict<K, V> *dict, size_type index,
    _DictIterator::pointer cur, bool in_rehash)
    : cur_(cur), dict_(dict),
      index_(index), in_rehash_(in_rehash) {
  dict_->acquireIterator(this);
}
template<class K, class V>
_DictIterator<K, V>::~_DictIterator() {
  if (cur_ != nullptr) {
    dict_->releaseIterator();
  }
}
template<class K, class V>
typename _DictIterator<K, V>::iterator &
_DictIterator<K, V>::operator++() {
  _DictTable<K, V> &table =
      *(dict_->isRehashing() ?
        dict_->rehash_ : dict_->data_);
  if (cur_ != nullptr) {
    cur_ = cur_->next;
  }
  while (cur_ == nullptr) {
    index_++;
    if (index_ >= table.capacity_) {
      if (!in_rehash_ && dict_->isRehashing()) {
        in_rehash_ = true;
        table = *(dict_->rehash_);
        index_ = 0;
      } else {
        cur_ = nullptr;
        break;
      }
    }
    cur_ = table.at(index_);
  }
  return *this;
}
template<class K, class V>
typename _DictIterator<K, V>::iterator // NOLINT
_DictIterator<K, V>::operator++(int) {
  iterator it(*this);
  ++(*this);
  return it;
}

template<class K, class V>
typename Dict<K, V>::size_type
Dict<K, V>::fixSize(Dict<K, V>::size_type n) const {
  size_type i = kInitialSize;
  if (n >= SIZE_MAX) { return SIZE_MAX + 1U; }
  for (; i < n; i *= 2);
  return i;
}
template<class K, class V>
template<class T>
T Dict<K, V>::reverseBit(T n) const {
  T s = 8 * sizeof(n),
      mask = ~T();
  while ((s >>= 1u) > 0) {
    mask ^= (mask << s);
    n = ((n >> s) & mask) | ((n << s) & ~mask);
  }
  return n;
}

template<class K, class V>
typename Dict<K, V>::iterator
Dict<K, V>::findIterIndex(const K &key) {
  if (empty()) {
    return iterator(this);
  }
  if (isRehashing()) { rehash(); }
  _DictTable<K, V> *table = data_;
  for (size_type i = 0; i < 1 + isRehashing(); i++, table = rehash_) {
    size_type idx = rd::hash<K>()(key) & table->mask();
    pointer entry = table->at(idx);
    for (; entry != nullptr; entry = entry->next) {
      if (entry->key == key) {
        return iterator(this, idx, entry, i == 1);
      }
    }
//    if (!isRehashing()) { break; }
  }
  return iterator(this);
}
template<class K, class V>
typename Dict<K, V>::iterator Dict<K, V>::setKeyValue(
    _DictIterator<K, V> iter, K &key, V &value) {
  // new entry
  pointer entry = nullptr;
  _DictTable<K, V> *table =
      isRehashing() ? rehash_ : data_;
  // key was in table. Need to iterate and find it again
  // because of the single direction linked list.
  if (iter != nullptr) {
    _DictTable<K, V> *iter_table =
        iter.in_rehash_ ? rehash_ : data_;
    pointer prev = nullptr,
        cur = iter_table->at(iter.index_);
    for (; cur != nullptr; cur = cur->next) {
      if (cur->key == iter->key) {
        if (prev != nullptr) {
          prev->next = cur->next;
        } else {
          iter_table->at(iter.index_) = cur->next;
        }
        entry = cur;
        entry->value = std::move(value);
        break;
      }
      prev = cur;
    }
  } else {
    entry = new _DictEntry<K, V>(key, value);
    table->size_++;
  }
  // entry cannot be nullptr since iter != nullptr
  assert(entry != nullptr);
  size_type idx = hash<K>()(key) & table->mask();
  entry->next = table->at(idx);
  table->at(idx) = entry;
  expand();
  return iterator(this, idx, entry, table == rehash_);
}
template<class K, class V>
void Dict<K, V>::rehash(size_type n) {
  // Note that iterator is not safe rehashing.
  if (!isRehashable() || !isRehashing()) { return; }
  for (size_type i = 0; i != n && data_->size_ != 0; i++) {
    size_type visited_buckets = 0;
    // Note that process_ can't overflow as there are
    // more elements because data->size_ != 0
    assert(data_->capacity_ > process_);
    while (data_->at(process_) == nullptr) {
      process_++, visited_buckets++;
      if (visited_buckets == kRehashSliceLength) { return; }
    }
    // Move all the elements from the old bucket to the new.
    for (pointer entry = data_->at(process_);
         entry != nullptr;) {
      pointer next_entry = entry->next;
      size_type index = hash<K>()(entry->key)
          & rehash_->mask();
      entry->next = rehash_->at(index);
      rehash_->at(index) = entry;
      data_->size_--;
      rehash_->size_++;
      entry = next_entry;
    }
    data_->at(process_) = nullptr;
    process_++;
  }
  if (data_->size_ == 0) {
    stopRehash();
  }
}

// start rehash
template<class K, class V>
void Dict<K, V>::resize(size_type n) {
  if (isRehashing() || data_->size_ > n) { return; }
  size_type real_size = fixSize(n);
  if (real_size == data_->capacity_) { return; }
//  rehash_ = make_shared<_DictTable<K, V>>(real_size);
  assert(rehash_ == nullptr);
  rehash_ = new _DictTable<K, V>(real_size);
  // Note that do not need to set process_ to 0,
  // since it has been set to 0 in stopRehash and constructor.
}

template<class K, class V>
void Dict<K, V>::stopRehash() {
  std::swap(data_, rehash_);
  delete (rehash_);
  rehash_ = nullptr;
  process_ = 0;
}

template<class K, class V>
void Dict<K, V>::acquireIterator(iterator *iter) {
  if (*iter != nullptr) {
    iter_num_++;
  }
}
template<class K, class V>
void Dict<K, V>::releaseIterator() {
  iter_num_--;
}

template<class K, class V>
Dict<K, V>::Dict()
    : process_(0), iter_num_(0), resizable(true) {
  rehash_ = nullptr;
  data_ = new _DictTable<K, V>(kInitialSize);
}
template<class K, class V>
Dict<K, V>::~Dict() {
  delete (data_);
  delete (rehash_);
}

template<class K, class V>
typename Dict<K, V>::iterator Dict<K, V>::begin() {
  iterator it(this, 0, data_->at(0));
  return ++it;
}
template<class K, class V>
typename Dict<K, V>::iterator Dict<K, V>::end() {
  return iterator(this);
}
template<class K, class V>
typename Dict<K, V>::size_type Dict<K, V>::size() const {
  return data_->size_ +
      (isRehashing() ? rehash_->size_ : 0);
}
template<class K, class V>
bool Dict<K, V>::empty() const {
  return data_->size_ == 0 &&
      (isRehashing() ? rehash_->size_ == 0 : true);
}

template<class K, class V>
void Dict<K, V>::shrink() {
  if (!resizable || isRehashing()) { return; }
  size_type minimal = data_->size_;
  if (minimal < kInitialSize) {
    minimal = kInitialSize;
  }
  return resize(minimal);
}
template<class K, class V>
void Dict<K, V>::expand() {
  if (isRehashing()) { return; }
  if ((data_->size_ >= kResizeRatio * data_->capacity_) &&
      (resizable || (data_->size_ >=
          data_->capacity_ * kForceResizeRatio))) {
    resize(data_->size_ * 2);
  }
}

template<class K, class V>
typename Dict<K, V>::iterator
Dict<K, V>::get(const K &key) {
  return findIterIndex(key);
}
template<class K, class V>
typename Dict<K, V>::iterator
Dict<K, V>::add(K &key, V &value) {
  if (isRehashing()) { rehash(); }
  iterator iter = findIterIndex(key);
  if (iter != nullptr) {
    return iterator(this);
  }
  return setKeyValue(iter, key, value);
}
template<class K, class V>
typename Dict<K, V>::iterator
Dict<K, V>::replace(const K &key, V &value) {
  if (isRehashing()) { rehash(); }
  return setKeyValue(findIterIndex(key), key, value);
}
template<class K, class V>
bool Dict<K, V>::remove(const K &key) {
  if (empty()) {
    return false;
  }
  if (isRehashing()) { rehash(); }
  _DictTable<K, V> *table = data_;
  for (size_type i = 0; i < 1 + isRehashing(); i++, table = rehash_) {
    size_type idx = hash<K>()(key) & table->mask();
    for (pointer prev = nullptr, cur = table->at(idx);
         cur != nullptr; cur = cur->next) {
      if (cur->key == key) {
        if (prev != nullptr) {
          prev->next = cur->next;
        } else {
          table->at(idx) = cur->next;
        }
        delete (cur);
        table->size_--;
        return true;
      }
      prev = cur;
    }
  }
  return false;
}
template<class K, class V>
template<class UnFn>
size_type Dict<K, V>::scan(size_type n, UnFn fn) {
  if (empty()) { return 0; }
  if (isRehashing()) {
    _DictTable<K, V> *foo = data_, *bar = rehash_;
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
    _DictTable<K, V> *table = data_;
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
