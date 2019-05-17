//
// Created by suun on 5/17/19.
//

// cassert for assert
// chrono for time diff
// functional for std::hash
#include <cassert>
#include <chrono>
#include <functional>
#include "dict.h"

namespace rd {
_DictEntry::_DictEntry(Var &k, Var &v) : next() {
  key = k;
  value = v;
}

_DictTable::_DictTable(size_t n)
    : capacity_(n), size_(0) {
  table_ = vector<_DictEntry *>(n);
}

_DictTable::~_DictTable() {
  for (_DictEntry *entry : table_) {
    while (entry != nullptr) {
      _DictEntry *tmp = entry;
      entry = entry->next;
//      allocator_.destroy(tmp);
//      allocator_.deallocate(tmp, 1);
      delete tmp;
    }
  }
}

_DictIterator::_DictIterator(const _DictIterator &it)
    : cur_(it.cur_), dict_(it.dict_),
      index_(it.index_), in_rehash_(it.in_rehash_) {
  dict_->acquireIterator(this);
}

_DictIterator::_DictIterator(
    Dict *dict, size_type index,
    _DictIterator::pointer cur, bool in_rehash)
    : cur_(cur), dict_(dict),
      index_(index), in_rehash_(in_rehash) {
  dict_->acquireIterator(this);
}

_DictIterator::~_DictIterator() {
  if (cur_ != nullptr) {
    dict_->releaseIterator();
  }
}

typename _DictIterator::iterator &
_DictIterator::operator++() {
  _DictTable &table =
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

typename _DictIterator::iterator // NOLINT
_DictIterator::operator++(int) {
  iterator it(*this);
  ++(*this);
  return it;
}

typename Dict::size_type
Dict::fixSize(Dict::size_type n) const {
  size_type i = kInitialSize;
  if (n >= SIZE_MAX) { return SIZE_MAX + 1U; }
  for (; i < n; i *= 2);
  return i;
}

typename Dict::iterator
Dict::findIterIndex(const typename Dict::key_type &key) {
  if (empty()) {
    return iterator(this);
  }
  if (isRehashing()) { rehash(); }
  _DictTable *table = data_;
  for (size_type i = 0; i < 1 + isRehashing(); i++, table = rehash_) {
    size_type idx = rd::hash<key_type>()(key) & table->mask();
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

typename Dict::iterator Dict::setKeyValue(
    const iterator &iter, key_type &key, value_type &value) {
  // new entry
  pointer entry = nullptr;
  _DictTable *table =
      isRehashing() ? rehash_ : data_;
  // key was in table. Need to iterate and find it again
  // because of the single direction linked list.
  if (iter != nullptr) {
    _DictTable *iter_table =
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
        entry->value = value;
        break;
      }
      prev = cur;
    }
  } else {
//    entry = table->allocator_.allocate(1);
//    table->allocator_.construct(entry, key, value);
    entry = new _DictEntry(key, value);
    table->size_++;
  }
  // entry cannot be nullptr since iter != nullptr
  assert(entry != nullptr);
  size_type idx = hash<key_type>()(key) & table->mask();
  entry->next = table->at(idx);
  table->at(idx) = entry;
  expand();
  return iterator(this, idx, entry, table == rehash_);
}

void Dict::rehash(size_type n) {
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
      size_type index = hash<key_type>()(entry->key)
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

void Dict::resize(size_type n) {
  if (isRehashing() || data_->size_ > n) { return; }
  size_type real_size = fixSize(n);
  if (real_size == data_->capacity_) { return; }
//  rehash_ = make_shared<_DictTable>(real_size);
  assert(rehash_ == nullptr);
//  rehash_ = allocator_.allocate(1);
//  allocator_.construct(rehash_, real_size);
  rehash_ = new _DictTable(real_size);
  // Note that do not need to set process_ to 0,
  // since it has been set to 0 in stopRehash and constructor.
}

void Dict::stopRehash() {
  std::swap(data_, rehash_);
//  allocator_.destroy(rehash_);
//  allocator_.deallocate(rehash_, 1);
  delete rehash_;
  rehash_ = nullptr;
  process_ = 0;
}

void Dict::acquireIterator(iterator *iter) {
  if (*iter != nullptr) {
    iter_num_++;
  }
}

void Dict::releaseIterator() {
  iter_num_--;
}

Dict::Dict()
    : process_(0), iter_num_(0), resizable(true) {
  rehash_ = nullptr;
//  data_ = allocator_.allocate(1);
//  allocator_.construct(data_, kInitialSize);
  data_ = new _DictTable(kInitialSize);
}

Dict::~Dict() {
//  allocator_.destroy(data_);
//  allocator_.deallocate(data_, 1);
//    allocator_.destroy(rehash_);
//    allocator_.deallocate(rehash_, 1);
  delete data_;
  delete rehash_;
}

typename Dict::iterator Dict::begin() {
  iterator it(this, 0, data_->at(0));
  return ++it;
}

typename Dict::iterator Dict::end() {
  return iterator(this);
}

typename Dict::size_type Dict::size() const {
  return data_->size_ +
      (isRehashing() ? rehash_->size_ : 0);
}

bool Dict::empty() const {
  return data_->size_ == 0 &&
      (isRehashing() ? rehash_->size_ == 0 : true);
}

void Dict::rehashMilliseconds(rd::size_type n) {
  std::chrono::time_point<std::chrono::system_clock> end =
      std::chrono::system_clock::now() + std::chrono::milliseconds(n);
  while (std::chrono::system_clock::now() < end) {
    if (isRehashing()) {
      rehash(kRehashMsDuration);
    } else {
      break;
    }
  }
}
void Dict::shrink() {
  if (!resizable || isRehashing()) { return; }
  size_type minimal = data_->size_;
  if (minimal < kInitialSize) {
    minimal = kInitialSize;
  }
  return resize(minimal);
}

void Dict::expand() {
  if (isRehashing()) { return; }
  if ((data_->size_ >= kResizeRatio * data_->capacity_) &&
      (resizable || (data_->size_ >=
          data_->capacity_ * kForceResizeRatio))) {
    resize(data_->size_ * 2);
  }
}

typename Dict::iterator
Dict::get(const key_type &key) {
  return findIterIndex(key);
}

typename Dict::iterator
Dict::add(key_type &key, value_type &value) {
  if (isRehashing()) { rehash(); }
  iterator iter = findIterIndex(key);
  if (iter != nullptr) {
    return iterator(this);
  }
  return setKeyValue(iter, key, value);
}

typename Dict::iterator
Dict::replace(key_type &key, value_type &value) {
  if (isRehashing()) { rehash(); }
  return setKeyValue(findIterIndex(key), key, value);
}

bool Dict::remove(const key_type &key) {
  if (empty()) {
    return false;
  }
  if (isRehashing()) { rehash(); }
  _DictTable *table = data_;
  for (size_type i = 0; i < 1 + isRehashing(); i++, table = rehash_) {
    size_type idx = hash<key_type>()(key) & table->mask();
    for (pointer prev = nullptr, cur = table->at(idx);
         cur != nullptr; cur = cur->next) {
      if (cur->key == key) {
        if (prev != nullptr) {
          prev->next = cur->next;
        } else {
          table->at(idx) = cur->next;
        }
//        table->allocator_.destroy(cur);
//        table->allocator_.deallocate(cur, 1);
        delete cur;
        table->size_--;
        return true;
      }
      prev = cur;
    }
  }
  return false;
}

}