//
// Created by suun on 5/17/19.
//

#include <random>
#include "skiplist.h"

namespace rd {

_SkipListNode::_SkipListNode(int level_num, double score,
                             const String &elem = String())
    : elem(elem), score(score), prev(nullptr) {
  levels = new _SkipListLevel[level_num];
}
_SkipListNode::~_SkipListNode() { delete[] levels; }

_SkipListNode &_SkipListIterator::operator*() {
  return *node_;
}
_SkipListIterator::pointer _SkipListIterator::operator->() {
  return &operator*();
}
_SkipListIterator &_SkipListIterator::operator++() {
  node_ = node_->levels[0].next;
  return *this;
}
const _SkipListIterator _SkipListIterator::operator++(int) {
  _SkipListIterator iterator = *this;
  operator++();
  return iterator;
}
_SkipListIterator &_SkipListIterator::operator--() {
  node_ = node_->prev;
  return *this;
}
const _SkipListIterator _SkipListIterator::operator--(int) {
  _SkipListIterator iterator = *this;
  operator--();
  return iterator;
}
bool _SkipListIterator::operator==(const _SkipListIterator &it) const {
  return node_ == it.node_;
}
bool _SkipListIterator::operator!=(const _SkipListIterator &it) const {
  return !(operator==(it));
}

SkipList::size_type SkipList::randomLevel() {
  size_type level = 1;
  std::mt19937 gen((std::random_device()()));
  std::uniform_int_distribution<> distribution(0, 0xFFFF);
  while (distribution(gen) < (0xFFFFu >> kLevelShiftBits)) {
    level += 1;
  }
  return std::min(level, kMaxLevel);
}

SkipList::SkipList() : level_(1), size_(0), tail_(nullptr) {
  head_ = new _SkipListNode(kMaxLevel, 0);
}
SkipList::~SkipList() {
  _SkipListNode *node = head_->levels[0].next;
  while (node) {
    _SkipListNode *next = node->levels[0].next;
    delete node;
    node = next;
  }
}
SkipList::iterator SkipList::insert(const String &elem, double score) {
  link_type pos[kMaxLevel], node = head_;
  size_type rank[kMaxLevel];
  for (int i = level_ - 1; i >= 0; i--) {
    rank[i] = (i == level_ - 1) ? 0 : rank[i + 1];
    while (node->levels[i].next &&
        (node->levels[i].next->score < score ||
            (node->levels[i].next->score == score &&
                node->levels[i].next->elem < elem))) {
      rank[i] += node->levels[i].span;
      node = node->levels[i].next;
    }
    pos[i] = node;
  }
  size_type level = randomLevel();
  for (int i = level_; i < level; i++) {
    rank[i] = 0;
    pos[i] = head_;
    pos[i]->levels[i].span = size_;
  }
  node = new _SkipListNode(level, score, elem);
  for (int i = 0; i < level; i++) {
    _SkipListLevel &forward = pos[i]->levels[i];
    node->levels[i].next = forward.next;
    forward.next = node;
    node->levels[i].span = forward.span - (rank[0] - rank[i]);
    forward.span = rank[0] - rank[i] + 1;
  }
  for (int i = level; i < level_; i++) {
    pos[i]->levels[i].span++;
  }
  node->prev = (pos[0] == head_) ? nullptr : pos[0];
  if (node->levels[0].next) {
    node->levels[0].next->prev = node;
  } else {
    tail_ = node;
  }
  size_++;
  return iterator(node);
}

}  // namespace rd