//
// Created by suun on 5/13/19.
//

#ifndef REDIS_DICT_H
#define REDIS_DICT_H
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "sds.h"

namespace std {
template<>
struct hash<rd::String> {
  std::size_t operator()(const rd::String &key) const {
    return std::_Hash_impl::hash(key.data(), key.size());
  }
};
//template<>
//struct __is_fast_hash<hash<rd::String>> : std::false_type {};
}

namespace rd {

using std::hash;
using std::vector;

typedef size_t size_type;

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

  inline size_t capacityMask() const { return capacity_ - 1; }

//  _DictTable();
  explicit _DictTable(size_t n);
  ~_DictTable() = default;
  _DictEntry<K, V> *
  &at(size_type n) { return table_[n]; }
};

template<class K, class V>
class _DictIterator {
  friend class Dict<K, V>;

 public:
  typedef size_t size_type;
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
  typedef size_t size_type;
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

  iterator findIterIndex(const K &key);
  typename Dict<K, V>::iterator setKeyValue(
      _DictIterator<K, V> iter, K &key, V &value);
  void rehash(size_type n = 1);
  void resize(size_type n);

  inline void stopRehash();
  inline void acquireIterator();
  inline void releaseIterator();

 public:
  Dict();
  ~Dict() = default;

  iterator begin();
  iterator end();

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
_DictIterator<K, V>::_DictIterator(const _DictIterator &it)
    : cur_(it.cur_), dict_(it.dict_),
      index_(it.index_), in_rehash_(it.in_rehash_) {
  dict_->acquireIterator();
}
template<class K, class V>
_DictIterator<K, V>::_DictIterator(
    Dict<K, V> *dict, size_type index,
    _DictIterator::pointer cur, bool in_rehash)
    : cur_(cur), dict_(dict),
      index_(index), in_rehash_(in_rehash) {
  if (cur_ != nullptr) {
    dict_->acquireIterator();
  }
}
template<class K, class V>
_DictIterator<K, V>::~_DictIterator() {
  dict_->releaseIterator();
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
Dict<K, V>::fixSize(size_type n) const {
  size_type i = kInitialSize;
  if (n >= SIZE_MAX) { return SIZE_MAX + 1U; }
  for (; i < n; i *= 2);
  return i;
}

template<class K, class V>
typename Dict<K, V>::iterator
Dict<K, V>::findIterIndex(const K &key) {
  size_type idx = 0;
  if (data_->size_ == 0 && rehash_->size_ == 0) {
    return iterator(this);
  }
  if (isRehashing()) { rehash(); }
  _DictTable<K, V> *table = data_;
  for (int i = 0; i < 2; i++, table = rehash_) {
    idx = rd::hash<K>()(key) & table->capacityMask();
    pointer entry = table->at(idx);
    for (; entry != nullptr; entry = entry->next) {
      if (entry->key == key) {
        return iterator(this, idx, entry, i == 1);
      }
    }
    if (!isRehashing()) { break; }
  }
  return iterator(this);
}
template<class K, class V>
typename Dict<K, V>::iterator Dict<K, V>::setKeyValue(
    _DictIterator<K, V> iter, K &key, V &value) {
  // new entry
  pointer entry = nullptr;
  // key was in table. Need to iterate and find it again
  // because of the single direction linked list.
  if (iter != nullptr) {
    _DictTable<K, V> *iter_table =
        iter.in_rehash_ ? rehash_ : data_;
    pointer prev = nullptr,
        cur = iter_table->at(iter.index_);
    for (; cur != nullptr; cur = cur->next) {
      if (cur == iter.cur_) {
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
  }
  // entry cannot be nullptr since iter != nullptr
  assert(entry != nullptr);
  _DictTable<K, V> *table =
      isRehashing() ? rehash_ : data_;
  size_type idx = hash<K>()(key) & table->capacityMask();
  entry->next = table->at(idx);
  table->at(idx) = entry;
  return iterator(this, idx, entry, table == rehash_);
}
template<class K, class V>
void Dict<K, V>::rehash(size_type n) {
  // Note that iterator is not safe rehashing.
  if (!isRehashable() || !isRehashing()) { return; }
  for (int i = 0; i != n && data_->size_ != 0; i++) {
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
          & rehash_->capacityMask();
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
  if (real_size == n) { return; }
//  rehash_ = make_shared<_DictTable<K, V>>(real_size);
  delete (rehash_);
  rehash_ = new _DictTable<K, V>(real_size);
  // Note that do not need to set process_ to 0,
  // since it has been set to 0 in stopRehash and constructor.
}

template<class K, class V>
void Dict<K, V>::stopRehash() {
  std::swap(data_, rehash_);
  delete (rehash_);
  assert(rehash_ == nullptr);
  process_ = 0;
}

template<class K, class V>
void Dict<K, V>::acquireIterator() {
  iter_num_++;
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
typename Dict<K, V>::iterator Dict<K, V>::begin() {
  return iterator(this, 0, data_->at(0));
}
template<class K, class V>
typename Dict<K, V>::iterator Dict<K, V>::end() {
  return iterator(this);
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
  if (isRehashing()) { rehash();}
  return setKeyValue(findIterIndex(key), key, value);
}
template<class K, class V>
bool Dict<K, V>::remove(const K &key) {
  return false;
}
template<class K, class V>
template<class UnFn>
size_type Dict<K, V>::scan(size_type n, UnFn fn) {
  return 0;
}
}

#endif //REDIS_DICT_H
