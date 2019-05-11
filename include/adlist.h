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
  explicit ListNode(T val = T(), ListNode *pv = nullptr, ListNode *nt = nullptr) : value(val), prev(pv), next(nt) {}
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

 private:
  link_type _node;

 public:
  ListIterator() : _node(nullptr) {}
  ListIterator(const ListIterator &it) : _node(it._node) {}
  explicit ListIterator(const link_type &node) : _node(node) {}

  reference operator*() {
    return _node->value;
  }

  pointer operator->() {
    return &(operator*());
  }

  ListIterator &operator++() {
    _node = _node->next;
    return *this;
  }

  const ListIterator operator++(int) {
    ListIterator tmp = *this;
    operator++();
    return tmp;
  }

  ListIterator &operator--() {
    _node = _node->prev;
  }

  const ListIterator operator--(int) {
    ListIterator tmp = *this;
    operator--();
    return tmp;
  }

  bool operator==(const ListIterator &it) const {
    return _node == it._node;
  }

  bool operator!=(const ListIterator &it) const {
    return !(operator==(it));
  }

};

template<class T>
class List {
 public:
  typedef T value_type;
  typedef ListIterator<T> iterator;
  typedef ListIterator<T> const_iterator;
  typedef std::reverse_iterator<ListIterator<T>> reverse_iterator;
  typedef ListNode<T> *link_type;
  typedef T &reference;
  typedef const T &const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

 private:
  link_type _tail;
  size_type _len{};

  void _empty_initialize() {
    _tail = new ListNode<T>();
    _tail->next = _tail;
    _tail->prev = _tail;
    _len = 0;
  }

  template<class InIt>
  void _list_aux(const InIt first, const InIt last, std::__false_type) {
    _empty_initialize();
    for (auto ptr = first; ptr != last; ptr++)
      _add_node(_tail, *ptr);
  }

  void _list_aux(const size_type n, const_reference val, std::__true_type) {
    _empty_initialize();
    for (size_type i = 0; i < n; i++)
      _add_node(_tail, val);
  }

  void _add_node(link_type node, const_reference val) {
    auto pNode = new ListNode<T>(val, node->prev, node);
    node->prev->next = pNode;
    node->prev = pNode;
    _len++;
  }

  link_type _remove_node(link_type node) {
    link_type next = node->next;
    link_type prev = node->prev;
    next->prev = prev;
    prev->next = next;
    delete node;
    _len--;
    return next;
  }

  link_type _erase(link_type first, link_type last) {
    while (first != last)
      first = _remove_node(first);
  }

 public:
  List() {
    _empty_initialize();
  }
  List(const List &list) {
//    _empty_initialize();
    _list_aux(list.begin(), list.end(), std::__false_type());
  }

  List copy() {
    List pList;
    pList._tail = _tail;
    pList._len = _len;
    return pList;
  }

  ~List() {
    _erase(_tail->next, _tail);
    delete (_tail);
  }

  template<class InIt>
  List(const InIt first, const InIt last) {
    _list_aux(first, last, typename std::__is_integer<InIt>::__type());
  }
  List(size_type n, const_reference val) {
    _list_aux(n, val, typename std::__is_integer<size_type>::__type());
  }

  List(std::initializer_list<T> list) {
    _list_aux(list.begin(), list.end(), typename std::__false_type());
  }

  List &merge(List &list) {
    _tail->prev->next = list._tail->next;
    list._tail->prev->next = _tail;
    _len += list._len;
    list._tail->next = list._tail->prev = list._tail;
    list._len = 0;
    return *this;
  }

  size_type size() const { return _len; }
  iterator begin() { return iterator(_tail->next); }
  iterator end() { return iterator(_tail); }
  reverse_iterator rbegin() const { return reverse_iterator(end()); }
  reverse_iterator rend() const { return reverse_iterator(begin()); }
  const_iterator begin() const { return const_iterator(_tail->next); }
  const_iterator end() const { return const_iterator(_tail); }

  void push_back(const_reference val) {
    _add_node(_tail, val);
  }
  void push_front(const_reference val) {
    _add_node(_tail->next, val);
  }
  value_type pop_back() {
    value_type val = _tail->prev->value;
    _remove_node(_tail->prev);
    return val;
  }
  value_type pop_front() {
    value_type val = _tail->next->value;
    _remove_node(_tail->next);
    return val;
  }

  iterator search(const_reference val) {
    return std::find_if(_tail->next, _tail, [=](link_type node) { return node->value == val; });
  }

  void rotate() {
    value_type val = pop_back();
    push_front(val);
  }

  value_type operator[](const size_type n) {
    iterator pit = _tail->next;
    for (size_type i = n; i != 0; i--) {
      pit++;
    }
    return *pit;
  }

  List &operator=(const List &list) {
    _erase(_tail->next, _tail);
    _list_aux(list.begin(), list.end(), std::__false_type());
    return *this;
  }
};
}

#endif //REDIS_ADLIST_H
