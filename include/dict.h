//
// Created by suun on 5/13/19.
//

#ifndef REDIS_DICT_H
#define REDIS_DICT_H
#include <cassert>
#include <string>
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

namespace {

using std::hash;
using std::make_shared;
using std::shared_ptr;
using std::unique_ptr;
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
  shared_ptr<_DictEntry> next;
  _DictEntry(K &k, V &v);
};

template<class K, class V>
struct _DictTable {
  vector<shared_ptr<_DictEntry<K, V>>> table_;
  size_type capacity_;
  size_type size_;

  inline size_t capacityMask() const { return capacity_ - 1; }

//  _DictTable();
  explicit _DictTable(size_t n);
  ~_DictTable() = default;
};

template<class K, class V>
class _DictIterator {
 public:
  typedef size_t size_type;
  typedef _DictIterator<K, V> iterator;
  typedef std::forward_iterator_tag iterator_category;
  typedef _DictEntry<K, V> entry_type;
  typedef Dict<K, V> dict_type;
  typedef shared_ptr<Dict<K, V>> dict_pointer;
  typedef entry_type &reference;
  typedef shared_ptr<entry_type> pointer;

 private:
  pointer cur_;
  dict_pointer dict_;
  size_type index_;

 public:
  _DictIterator()
      : cur_(nullptr), dict_(nullptr), index_(0) {}
  _DictIterator(const _DictIterator &it);
  explicit _DictIterator(dict_pointer dict, pointer cur = nullptr);
  ~_DictIterator();

  reference operator*()
  const { return *cur_; }

  reference operator->()
  const { return &(operator*()); }

  iterator &operator++();
  iterator operator++(int) const; // NOLINT

  bool operator==(const iterator &it)
  const { return dict_ == it.dict_ && cur_ == it.cur_; }
  bool operator!=(const iterator &it)
  const { return !(operator==(it)); }
};

template<class K, class V>
class Dict {
  friend class _DictIterator<K, V>;
 public:
  typedef K key_type;
  typedef V value_type;
  typedef size_t size_type;
  typedef _DictIterator<K, V> iterator;
  typedef _DictEntry<K, V> entry_type;
  typedef shared_ptr<entry_type> pointer;
  typedef entry_type &reference;

 private:
  static const size_type kInitialSize = 4;
  static const size_type kRehashSliceLength = 10;

  shared_ptr<_DictTable<K, V>> data_, rehash_;
  size_type process_;
  size_type iter_num_;

  void rehash(size_type n = 1);
  inline void startRehash();
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

  iterator get(const K &key);
  iterator add(const K &key, const V &value);
  iterator replace(const K &key, const V &value);
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
  table_ = vector<shared_ptr<_DictEntry<K, V>>>(n);
}
template<class K, class V>
_DictIterator<K, V>::_DictIterator(const _DictIterator &it)
    : cur_(it.cur_), dict_(it.dict_), index_(it.index_) {
  dict_->acquireIterator();
}
template<class K, class V>
_DictIterator<K, V>::_DictIterator(
    _DictIterator::dict_pointer dict, _DictIterator::pointer cur)
    : cur_(cur), dict_(dict), index_(0) {
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
      if (!dict_->isRehashing() && dict_->isRashing()) {
        table = *(dict_->rehash_);
        index_ = 0;
      } else {
        cur_ = nullptr;
        break;
      }
    }
    cur_ = table[index];
  }
  return *this;
}
template<class K, class V>
typename _DictIterator<K, V>::iterator // NOLINT
_DictIterator<K, V>::operator++(int) const {
  iterator it(*this);
  ++(*this);
  return it;
}

template<class K, class V>
void Dict<K, V>::rehash(size_type n) {
  if (!isRehashable() || !isRehashing()) { return; }
  for (int i = 0; i != n && data_->size_ != 0; i++) {
    size_type visited_buckets = 0;
    for (;
        data_->table_[process_] == nullptr ||
            visited_buckets != kRehashSliceLength;
        visited_buckets++,
            process_++);
//    if (visited_buckets == kRehashSliceLength) { return; }
    for (pointer entry = data_->table_[process_];
         entry != nullptr;) {
      pointer next_entry = entry->next;
      size_type index = hash<K>(entry->key)
          & rehash_->capacityMask();
      entry->next = rehash_->table_[index];
      rehash_->table_[index] = entry;
      data_->size_--;
      rehash_->size_++;
      entry = next_entry;
    }
    data_->table_[process_] = nullptr;
    process_++;
  }
}

template<class K, class V>
void Dict<K, V>::startRehash() {
  rehash_ = make_shared<_DictTable<K, V>>();
}
template<class K, class V>
void Dict<K, V>::stopRehash() {
  std::swap(data_, rehash_);
  rehash_.reset();
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
    : data_(kInitialSize), rehash_(nullptr),
      process_(0), iter_num_(0) {}

template<class K, class V>
typename Dict<K, V>::iterator Dict<K, V>::begin() {
  return iterator(this, data_[0]);
}
template<class K, class V>
typename Dict<K, V>::iterator Dict<K, V>::end() {
  return iterator(this);
}

template<class K, class V>
typename Dict<K, V>::iterator
Dict<K, V>::get(const K &key) {
  if (data_->size_ == 0 && rehash_->size_ == 0) {
    return iterator(this);
  }
  if (isRehashing()) { rehash(); }
  shared_ptr<_DictTable<K, V>> table = data_;
  for (int i = 0; i < 2; i++, table = rehash_) {
    size_type idx = hash<K>(key) & table->capacityMask();
    pointer entry = table[idx];
    for (; entry != nullptr; entry = entry->next) {
      if (entry->key == key) {
        return iterator(this, entry);
      }
    }
    if (!isRehashing()) { break; }
  }
  return iterator(this);
}
template<class K, class V>
typename Dict<K, V>::iterator
Dict<K, V>::add(const K &key, const V &value) {
}
template<class K, class V>
typename Dict<K, V>::iterator
Dict<K, V>::replace(const K &key, const V &value) {
  return Dict::iterator();
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
