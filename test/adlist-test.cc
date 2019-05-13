//
// Created by suun on 10/12/2018.
//

#include "redis.h"
#include <list>
#include <gmock/gmock.h>

using namespace testing;

const std::vector<int> v = {1, 2, 5, 3};

TEST(adlist, len) {
  rd::List<int> l(v.begin(), v.end());
  ASSERT_THAT(l.size(), 4);
}

TEST(adlist, first) {
  rd::List<int> l(v.begin(), v.end());
  ASSERT_THAT(*l.begin(), 1);
}

TEST(adlist, last) {
  rd::List<int> l(v.begin(), v.end());
  ASSERT_THAT(*(--l.end()), 3);
}

TEST(adlist, iterator) {
//  rd::List<int> l1;
//  ASSERT_THAT(l1.size(), 0);
  rd::List<int> l1 = rd::List<int>(v.begin(), v.end());
  ASSERT_THAT(l1.size(), 4);
//  for(auto it = l1.begin(); it != l1.end(); ++it, ++vit)
//    ASSERT_THAT(*it, *vit);
  ASSERT_THAT(l1, ElementsAreArray(v));
  auto vit = --v.end();
  for (auto it = l1.rbegin(); it != l1.rend(); ++it, --vit)
    ASSERT_THAT(*it, *vit);
}

TEST(adlist, push) {
  rd::List<int> l(v.begin(), v.end());
  l.push_front(3);
  ASSERT_THAT(l, ElementsAre(3, 1, 2, 5, 3));
  l.push_back(4);
  ASSERT_THAT(l, ElementsAre(3, 1, 2, 5, 3, 4));
}

TEST(adlist, insert) {
  rd::List<int> l(v.begin(), v.end());
  l.insert(2, 7, false);
  ASSERT_THAT(l, ElementsAre(1, 7, 2, 5, 3));
  l.insert(3, 0, true);
  ASSERT_THAT(l, ElementsAre(1, 7, 2, 5, 3, 0));
}

TEST(adlist, index) {
  rd::List<int> l(v.begin(), v.end());
  for (int i = 0; i < l.size(); i++) {
    ASSERT_THAT(l[i], v[i]);
  }
}

TEST(adlist, remove) {
  rd::List<int> l(v.begin(), v.end());
  l.remove(5);
  ASSERT_THAT(l, ElementsAre(1, 2, 3));
  l.remove(1);
  ASSERT_THAT(l, ElementsAre(2, 3));
  l.remove(3);
  ASSERT_THAT(l, ElementsAre(2));
  ASSERT_THAT(l.empty(), false);
  l.remove(2);
  ASSERT_THAT(l, ElementsAre());
  ASSERT_THAT(l.empty(), true);
}

TEST(adlist, rotate) {
  rd::List<int> l(v.begin(), v.end());
  l.rotate();
  ASSERT_THAT(l, ElementsAre(3, 1, 2, 5));
  l.rotate();
  ASSERT_THAT(l, ElementsAre(5, 3, 1, 2));
}
