//
// Created by suun on 5/15/19.
//

#include <gmock/gmock.h>
#include "redis.h"

using namespace testing;

TEST(dict, constructor) {
  std::shared_ptr<rd::Dict<int, int>> dict =
      std::make_shared<rd::Dict<int, int>>();
  dict->get(4);
  rd::_DictIterator<int, int> it(dict->begin());
  dict->shrink();
  dict->expand();
  int k = 2, j = 3;
  {
    dict->add(k, j);
    dict->get(k);
    it++;
    ASSERT_THAT(it, dict->end());
  }
  const int len = 1000;
  for (int i = 0; i < len; i++) {
    dict->add(i, j);
  }
  for (int i = 0; i < len; i += 5) {
    dict->remove(i);
  }
  for (int i = 0; i < len; i++) {
    if (i % 5 == 0) {
      ASSERT_THAT(dict->get(i), nullptr);
    } else {
      ASSERT_THAT(dict->get(i), Not(nullptr));
    }
  }
  rd::size_type cursor = 0, count = 0;
  do {
    cursor = dict->scan(
        cursor,
        [&count](rd::Dict<int, int>::pointer entry) {
          count++;
        }
    );
  } while (cursor);
  ASSERT_THAT(count, dict->size());
}
