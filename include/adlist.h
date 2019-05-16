//
// Created by suun on 10/9/2018.
//

#ifndef REDIS_ADLIST_H
#define REDIS_ADLIST_H

// For std::find
#include <algorithm>
#include "common.h"

namespace rd {
template<class T>
class _ListNode {
 public:
  _ListNode *prev;
  _ListNode *next;
  T value;
//  _ListNode() : value(T()), prev(nullptr), next(nullptr) {}
  explicit _ListNode(
      T val = T(), _ListNode *pv = nullptr, _ListNode *nt = nullptr);
};

template<class T>
class _ListIterator {
 public:
  typedef T value_type;
  typedef T *pointer;
  typedef T &reference;
  typedef std::bidirectional_iterator_tag iterator_category;

  typedef _ListNode<T> *link_type;
  typedef rd::size_type size_type;
  typedef ptrdiff_t difference_type;

  link_type node_;

 public:
  _ListIterator() : node_(nullptr) {}
  _ListIterator(const _ListIterator &it) : node_(it.node_) {}
  explicit _ListIterator(const link_type &node) : node_(node) {}

  reference operator*();
  pointer operator->();
  _ListIterator &operator++();
  const _ListIterator operator++(int);
  _ListIterator &operator--();
  const _ListIterator operator--(int);
  bool operator==(const _ListIterator &it) const;
  bool operator!=(const _ListIterator &it) const;
};

template<class T>
class _ListConstIterator {
 public:
  typedef T value_type;
  typedef const T *pointer;
  typedef const T &reference;
  typedef std::bidirectional_iterator_tag iterator_category;

  typedef const _ListNode<T> *link_type;
  typedef rd::size_type size_type;
  typedef ptrdiff_t difference_type;

  link_type node_;

 public:
  _ListConstIterator() : node_(nullptr) {}
  _ListConstIterator(const _ListConstIterator &it) : node_(it.node_) {}
  explicit _ListConstIterator(const _ListIterator<T> &it)
      : node_(it.node_) {}
  explicit _ListConstIterator(const link_type &node) : node_(node) {}

  reference operator*() const;
  pointer operator->() const;
  _ListConstIterator &operator++();
  const _ListConstIterator operator++(int);
  _ListConstIterator &operator--();
  const _ListConstIterator operator--(int);
  bool operator==(const _ListConstIterator &it) const;
  bool operator!=(const _ListConstIterator &it) const;
};

template<class T>
class List {
 public:
  typedef T value_type;
  typedef _ListIterator<T> iterator;
  typedef _ListConstIterator<T> const_iterator;
  typedef std::reverse_iterator<_ListIterator<T>> reverse_iterator;
  typedef std::reverse_iterator<_ListConstIterator<T>>
      const_reverse_iterator;
  typedef _ListNode<T> *link_type;
  typedef T &reference;
  typedef const T &const_reference;
  typedef rd::size_type size_type;
  typedef ptrdiff_t difference_type;

 private:
  link_type tail_;
  size_type len_{};

  void emptyInitialize();
  template<class InIt>
  void listAux(InIt first, InIt last, std::__false_type);
  void listAux(size_type n, const_reference val, std::__true_type);
  void addNode(link_type node, const_reference val);
  link_type removeNode(link_type node);
  link_type eraseRange(link_type first, link_type last);

 public:
  List();
  List(const List &list);
  List copy();
  ~List();

  template<class InIt>
  List(InIt first, InIt last);
  List(size_type n, const_reference val);
  List(std::initializer_list<T> list);

  bool empty() const;
  size_type size() const { return len_; }
  iterator begin() { return iterator(tail_->next); }
  iterator end() { return iterator(tail_); }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_iterator begin() const { return const_iterator(tail_->next); }
  const_iterator end() const { return const_iterator(tail_); }
  const_reverse_iterator rbegin()
  const { return const_reverse_iterator(rbegin()); }
  const_reverse_iterator rend()
  const { return const_reverse_iterator(rend()); }

  void pushBack(const_reference val);
  void pushFront(const_reference val);
  value_type popBack();
  value_type popFront();

  bool insert(const_reference pos, const_reference val, bool after);
  bool remove(const_reference val);
  iterator search(const_reference val);
  void rotate();
  List &merge(List &list);
  value_type operator[](size_type n);
  List &operator=(const List &list);
};
template<class T>
void List<T>::emptyInitialize() {
  tail_ = new _ListNode<T>();
  tail_->next = tail_;
  tail_->prev = tail_;
  len_ = 0;
}
template<class T>
template<class InIt>
void List<T>::listAux(const InIt first,
                      const InIt last,
                      std::__false_type) {
  emptyInitialize();
  for (auto ptr = first; ptr != last; ptr++)
    addNode(tail_, *ptr);
}
template<class T>
void List<T>::listAux(size_type n,
                      const_reference val,
                      std::__true_type) {
  emptyInitialize();
  for (size_type i = 0; i < n; i++)
    addNode(tail_, val);
}
template<class T>
void List<T>::addNode(List::link_type node, const_reference val) {
  auto pNode = new _ListNode<T>(val, node->prev, node);
  node->prev->next = pNode;
  node->prev = pNode;
  len_++;
}
template<class T>
typename List<T>::link_type List<T>::removeNode(List::link_type node) {
  link_type next = node->next;
  link_type prev = node->prev;
  next->prev = prev;
  prev->next = next;
  delete node;
  len_--;
  return next;
}
template<class T>
typename List<T>::link_type List<T>::eraseRange(
    List::link_type first, List::link_type last) {
  while (first != last)
    first = removeNode(first);
  return last;
}
template<class T>
List<T>::List() {
  emptyInitialize();
}

template<class T>
List<T>::List(const List &list) {
//    emptyInitialize();
  listAux(list.begin(), list.end(), std::__false_type());
}
template<class T>
List<T> List<T>::copy() {
  List pList;
  pList.tail_ = tail_;
  pList.len_ = len_;
  return pList;
}
template<class T>
List<T>::~List() {
  eraseRange(tail_->next, tail_);
  delete (tail_);
}
template<class T>
template<class InIt>
List<T>::List(const InIt first, const InIt last) {
  listAux(first, last, typename std::__is_integer<InIt>::__type());
}
template<class T>
List<T>::List(List::size_type n, const_reference val) {
  listAux(n, val, typename std::__is_integer<size_type>::__type());
}
template<class T>
List<T>::List(std::initializer_list<T> list) {
  listAux(list.begin(), list.end(), typename std::__false_type());
}
template<class T>
List<T> &List<T>::merge(List &list) {
  tail_->prev->next = list.tail_->next;
  list.tail_->prev->next = tail_;
  len_ += list.len_;
  list.tail_->next = list.tail_->prev = list.tail_;
  list.len_ = 0;
  return *this;
}
template<class T>
void List<T>::pushBack(const_reference val) {
  addNode(tail_, val);
}
template<class T>
void List<T>::pushFront(const_reference val) {
  addNode(tail_->next, val);
}
template<class T>
typename List<T>::value_type List<T>::popBack() {
  value_type val = tail_->prev->value;
  removeNode(tail_->prev);
  return val;
}
template<class T>
typename List<T>::value_type List<T>::popFront() {
  value_type val = tail_->next->value;
  removeNode(tail_->next);
  return val;
}
template<class T>
bool List<T>::insert(const_reference pos,
                     const_reference val,
                     bool after) {
  iterator it = std::find(begin(), end(), pos);
  if (it == end()) {
    return false;
  }
  if (after) { it++; }
  addNode(it.node_, val);
  return true;
}
template<class T>
bool List<T>::remove(const_reference val) {
  iterator it = std::find(begin(), end(), val);
  if (it == end()) {
    return false;
  }
  removeNode(it.node_);
  return true;
}
template<class T>
typename List<T>::iterator List<T>::search(const_reference val) {
  return std::find(begin(), end(), val);
}
template<class T>
void List<T>::rotate() {
  value_type val = popBack();
  pushFront(val);
}
template<class T>
bool List<T>::empty() const {
  return len_ == 0;
}
template<class T>
typename List<T>::value_type List<T>::operator[](size_type n) {
  iterator pit;
  if (n >= 0) {
    pit = iterator(tail_->next);
    for (size_type i = n; i != 0; i--) {
      pit++;
    }
  } else {
    pit = iterator(tail_);
    for (size_type i = n; i != 0; i++) {
      pit--;
    }
  }
  return *pit;
}
template<class T>
List<T> &List<T>::operator=(const List &list) {
  eraseRange(tail_->next, tail_);
  listAux(list.begin(), list.end(), std::__false_type());
  return *this;
}

template<class T>
_ListNode<T>::_ListNode(T val, _ListNode *pv, _ListNode *nt)
    : value(val), prev(pv), next(nt) {}

template<class T>
typename _ListIterator<T>::pointer _ListIterator<T>::operator->() {
  return &(operator*());
}
template<class T>
typename _ListIterator<T>::reference _ListIterator<T>::operator*() {
  return node_->value;
}
template<class T>
_ListIterator<T> &_ListIterator<T>::operator++() {
  node_ = node_->next;
  return *this;
}
template<class T>
const _ListIterator<T> _ListIterator<T>::operator++(int) {
  _ListIterator tmp = *this;
  operator++();
  return tmp;
}
template<class T>
_ListIterator<T> &_ListIterator<T>::operator--() {
  node_ = node_->prev;
  return *this;
}
template<class T>
const _ListIterator<T> _ListIterator<T>::operator--(int) {
  _ListIterator tmp = *this;
  operator--();
  return tmp;
}
template<class T>
bool _ListIterator<T>::operator==(const _ListIterator &it) const {
  return node_ == it.node_;
}
template<class T>
bool _ListIterator<T>::operator!=(const _ListIterator &it) const {
  return !(operator==(it));
}

template<class T>
typename _ListConstIterator<T>::reference
_ListConstIterator<T>::operator*() const {
  return node_->value;
}
template<class T>
typename _ListConstIterator<T>::pointer
_ListConstIterator<T>::operator->() const {
  return &(operator*());
}
template<class T>
_ListConstIterator<T> &_ListConstIterator<T>::operator++() {
  node_ = node_->next;
  return *this;
}
template<class T>
const _ListConstIterator<T> _ListConstIterator<T>::operator++(int) {
  _ListConstIterator tmp = *this;
  operator++();
  return tmp;
}
template<class T>
_ListConstIterator<T> &_ListConstIterator<T>::operator--() {
  node_ = node_->prev;
}
template<class T>
const _ListConstIterator<T> _ListConstIterator<T>::operator--(int) {
  _ListConstIterator tmp = *this;
  operator--();
  return tmp;
}
template<class T>
bool _ListConstIterator<T>::operator==(const _ListConstIterator &it) const {
  return node_ == it.node_;
}
template<class T>
bool _ListConstIterator<T>::operator!=(const _ListConstIterator &it) const {
  return !(operator==(it));
}

}

#endif //REDIS_ADLIST_H
