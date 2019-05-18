#include <experimental/memory_resource>
#include "common.h"
#include "sds.h"

#ifndef REDIS_SKIPLIST_H_
#define REDIS_SKIPLIST_H_
namespace rd {

struct _SkipListNode;

struct _SkipListLevel {
  _SkipListNode *next;
  size_type span;
  _SkipListLevel() : next(nullptr), span(0) {}
};

struct _SkipListNode {
  String elem;
  double score;
  _SkipListNode *prev;
  _SkipListLevel *levels;

  _SkipListNode(int level_num, double score, const String &elem);
  ~_SkipListNode();
};

class _SkipListIterator {
 public:
  typedef _SkipListNode value_type;
  typedef _SkipListNode *pointer;
  typedef _SkipListNode &reference;
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef pointer link_type;
  typedef rd::size_type size_type;

 private:
  link_type node_;

 public:
  _SkipListIterator() : node_(nullptr) {}
  _SkipListIterator(const _SkipListIterator &it) = default;
  explicit _SkipListIterator(link_type node) : node_(node) {}

  reference operator*();
  pointer operator->();
  _SkipListIterator &operator++();
  const _SkipListIterator operator++(int);
  _SkipListIterator &operator--();
  const _SkipListIterator operator--(int);
  bool operator==(const _SkipListIterator &it) const;
  bool operator!=(const _SkipListIterator &it) const;
};

class SkipList {
 public:
  typedef rd::size_type size_type;
  typedef _SkipListNode value_type;
  typedef _SkipListIterator iterator;
  typedef value_type *link_type;
  typedef value_type &reference;
  typedef value_type *pointer;

 private:
  link_type head_, tail_;
  size_type size_, level_;

  const size_type kMaxLevel = 64;
  const size_type kLevelShiftBits = 2;

  size_type randomLevel();

 public:
  SkipList();
  ~SkipList();

  iterator insert(const String &elem, double score);
};

}  // namespace rd

#endif
