//
// Created by suun on 10/9/2018.
//

#ifndef REDIS_ADLIST_H
#define REDIS_ADLIST_H

#include <memory>
#include <algorithm>
#include <iterator>

namespace rd {
template<class T>
class ListNode {
 public:
  ListNode *prev;
  ListNode *next;
  T value;
//  ListNode() : value(T()), prev(nullptr), next(nullptr) {}
  explicit ListNode(T val = T(), ListNode *pv = nullptr, ListNode *nt = nullptr);
};

template<class T>
class ListIterator {
 public:
  typedef T value_type;
  typedef T *pointer;
  typedef T &reference;
  typedef std::bidirectional_iterator_tag iterator_category;

  typedef ListNode<T> *link_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  link_type _node;

 public:
  ListIterator() : _node(nullptr) {}
  ListIterator(const ListIterator &it) : _node(it._node) {}
  explicit ListIterator(const link_type &node) : _node(node) {}

  reference operator*();
  pointer operator->();
  ListIterator &operator++();
  const ListIterator operator++(int);
  ListIterator &operator--();
  const ListIterator operator--(int);
  bool operator==(const ListIterator &it) const;
  bool operator!=(const ListIterator &it) const;
};

template<class T>
class ListConstIterator {
 public:
  typedef T value_type;
  typedef const T *pointer;
  typedef const T &reference;
  typedef std::bidirectional_iterator_tag iterator_category;

  typedef const ListNode<T> *link_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  link_type _node;

 public:
  ListConstIterator() : _node(nullptr) {}
  ListConstIterator(const ListConstIterator &it) : _node(it._node) {}
  explicit ListConstIterator(const ListIterator<T> &it) : _node(it._node) {}
  explicit ListConstIterator(const link_type &node) : _node(node) {}

  reference operator*() const;
  pointer operator->() const;
  ListConstIterator &operator++();
  const ListConstIterator operator++(int);
  ListConstIterator &operator--();
  const ListConstIterator operator--(int);
  bool operator==(const ListConstIterator &it) const;
  bool operator!=(const ListConstIterator &it) const;
};

template<class T>
class List {
 public:
  typedef T value_type;
  typedef ListIterator<T> iterator;
  typedef ListConstIterator<T> const_iterator;
  typedef std::reverse_iterator<ListIterator<T>> reverse_iterator;
  typedef std::reverse_iterator<ListConstIterator<T>> const_reverse_iterator;
  typedef ListNode<T> *link_type;
  typedef T &reference;
  typedef const T &const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

 private:
  link_type _tail;
  size_type _len{};

  void _empty_initialize();
  template<class InIt>
  void _list_aux(InIt first, InIt last, std::__false_type);
  void _list_aux(size_type n, const_reference val, std::__true_type);
  void _add_node(link_type node, const_reference val);
  link_type _remove_node(link_type node);
  link_type _erase(link_type first, link_type last);

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
  size_type size() const { return _len; }
  iterator begin() { return iterator(_tail->next); }
  iterator end() { return iterator(_tail); }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_iterator begin() const { return const_iterator(_tail->next); }
  const_iterator end() const { return const_iterator(_tail); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(rbegin()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(rend()); }

  void push_back(const_reference val);
  void push_front(const_reference val);
  value_type pop_back();
  value_type pop_front();

  bool insert(const_reference pos, const_reference val, bool after);
  bool remove(const_reference val);
  iterator search(const_reference val);
  void rotate();
  List &merge(List &list);
  value_type operator[](int n);
  List &operator=(const List &list);
};
template<class T>
void List<T>::_empty_initialize() {
  _tail = new ListNode<T>();
  _tail->next = _tail;
  _tail->prev = _tail;
  _len = 0;
}
template<class T>
template<class InIt>
void List<T>::_list_aux(const InIt first, const InIt last, std::__false_type) {
  _empty_initialize();
  for (auto ptr = first; ptr != last; ptr++)
    _add_node(_tail, *ptr);
}
template<class T>
void List<T>::_list_aux(const List::size_type n, const_reference val, std::__true_type) {
  _empty_initialize();
  for (size_type i = 0; i < n; i++)
    _add_node(_tail, val);
}
template<class T>
void List<T>::_add_node(List::link_type node, const_reference val) {
  auto pNode = new ListNode<T>(val, node->prev, node);
  node->prev->next = pNode;
  node->prev = pNode;
  _len++;
}
template<class T>
typename List<T>::link_type List<T>::_remove_node(List::link_type node) {
  link_type next = node->next;
  link_type prev = node->prev;
  next->prev = prev;
  prev->next = next;
  delete node;
  _len--;
  return next;
}
template<class T>
typename List<T>::link_type List<T>::_erase(List::link_type first, List::link_type last) {
  while (first != last)
    first = _remove_node(first);
}
template<class T>
List<T>::List() {
  _empty_initialize();
}

template<class T>
List<T>::List(const List &list) {
//    _empty_initialize();
  _list_aux(list.begin(), list.end(), std::__false_type());
}
template<class T>
List<T> List<T>::copy() {
  List pList;
  pList._tail = _tail;
  pList._len = _len;
  return pList;
}
template<class T>
List<T>::~List() {
  _erase(_tail->next, _tail);
  delete (_tail);
}
template<class T>
template<class InIt>
List<T>::List(const InIt first, const InIt last) {
  _list_aux(first, last, typename std::__is_integer<InIt>::__type());
}
template<class T>
List<T>::List(List::size_type n, const_reference val) {
  _list_aux(n, val, typename std::__is_integer<size_type>::__type());
}
template<class T>
List<T>::List(std::initializer_list<T> list) {
  _list_aux(list.begin(), list.end(), typename std::__false_type());
}
template<class T>
List<T> &List<T>::merge(List &list) {
  _tail->prev->next = list._tail->next;
  list._tail->prev->next = _tail;
  _len += list._len;
  list._tail->next = list._tail->prev = list._tail;
  list._len = 0;
  return *this;
}
template<class T>
void List<T>::push_back(const_reference val) {
  _add_node(_tail, val);
}
template<class T>
void List<T>::push_front(const_reference val) {
  _add_node(_tail->next, val);
}
template<class T>
typename List<T>::value_type List<T>::pop_back() {
  value_type val = _tail->prev->value;
  _remove_node(_tail->prev);
  return val;
}
template<class T>
typename List<T>::value_type List<T>::pop_front() {
  value_type val = _tail->next->value;
  _remove_node(_tail->next);
  return val;
}
template<class T>
bool List<T>::insert(const_reference pos, const_reference val, bool after) {
  iterator it = std::find(begin(), end(), pos);
  if (it == end()) {
    return false;
  }
  if (after) { it++; }
  _add_node(it._node, val);
  return true;
}
template<class T>
bool List<T>::remove(const_reference val) {
  iterator it = std::find(begin(), end(), val);
  if (it == end()) {
    return false;
  }
  _remove_node(it._node);
  return true;
}
template<class T>
typename List<T>::iterator List<T>::search(const_reference val) {
  return std::find(begin(), end(), val);
}
template<class T>
void List<T>::rotate() {
  value_type val = pop_back();
  push_front(val);
}
template<class T>
bool List<T>::empty() const {
  return _len == 0;
}
template<class T>
typename List<T>::value_type List<T>::operator[](const int n) {
  iterator pit;
  if (n >= 0) {
    pit = iterator(_tail->next);
    for (int i = n; i != 0; i--) {
      pit++;
    }
  } else {
    pit = iterator(_tail);
    for (int i = n; i != 0; i++) {
      pit--;
    }
  }
  return *pit;
}
template<class T>
List<T> &List<T>::operator=(const List &list) {
  _erase(_tail->next, _tail);
  _list_aux(list.begin(), list.end(), std::__false_type());
  return *this;
}

template<class T>
ListNode<T>::ListNode(T val, ListNode *pv, ListNode *nt)
    : value(val), prev(pv), next(nt) {}

template<class T>
typename ListIterator<T>::pointer ListIterator<T>::operator->() {
  return &(operator*());
}
template<class T>
typename ListIterator<T>::reference ListIterator<T>::operator*() {
  return _node->value;
}
template<class T>
ListIterator<T> &ListIterator<T>::operator++() {
  _node = _node->next;
  return *this;
}
template<class T>
const ListIterator<T> ListIterator<T>::operator++(int) {
  ListIterator tmp = *this;
  operator++();
  return tmp;
}
template<class T>
ListIterator<T> &ListIterator<T>::operator--() {
  _node = _node->prev;
}
template<class T>
const ListIterator<T> ListIterator<T>::operator--(int) {
  ListIterator tmp = *this;
  operator--();
  return tmp;
}
template<class T>
bool ListIterator<T>::operator==(const ListIterator &it) const {
  return _node == it._node;
}
template<class T>
bool ListIterator<T>::operator!=(const ListIterator &it) const {
  return !(operator==(it));
}

template<class T>
typename ListConstIterator<T>::reference ListConstIterator<T>::operator*() const {
  return _node->value;
}
template<class T>
typename ListConstIterator<T>::pointer ListConstIterator<T>::operator->() const {
  return &(operator*());
}
template<class T>
ListConstIterator<T> &ListConstIterator<T>::operator++() {
  _node = _node->next;
  return *this;
}
template<class T>
const ListConstIterator<T> ListConstIterator<T>::operator++(int) {
  ListConstIterator tmp = *this;
  operator++();
  return tmp;
}
template<class T>
ListConstIterator<T> &ListConstIterator<T>::operator--() {
  _node = _node->prev;
}
template<class T>
const ListConstIterator<T> ListConstIterator<T>::operator--(int) {
  ListConstIterator tmp = *this;
  operator--();
  return tmp;
}
template<class T>
bool ListConstIterator<T>::operator==(const ListConstIterator &it) const {
  return _node == it._node;
}
template<class T>
bool ListConstIterator<T>::operator!=(const ListConstIterator &it) const {
  return !(operator==(it));
}

}

#endif //REDIS_ADLIST_H
