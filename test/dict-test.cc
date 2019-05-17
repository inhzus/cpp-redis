//
// Created by suun on 5/15/19.
//

#include <gmock/gmock.h>
#include "redis.h"

using namespace testing;

using Var = std::variant<int, float>;

std::shared_ptr<rd::Dict> dict;
const int kLen = 9000;
Var ki(5), kj(10);
TEST(dict, constructor) {
  dict = std::make_shared<rd::Dict>();
}

TEST(dict, add) {
  for (Var i = 0; std::get<int>(i) < kLen; std::get<int>(i)++) {
    dict->add(i, i);
  }
  ASSERT_THAT(dict->size(), kLen);
  ASSERT_THAT(dict->get(ki)->value, ki);
  ASSERT_THAT(dict->get(kj)->value, kj);
  ASSERT_THAT(dict->isRehashing(), true);
}

TEST(dict, rehash) {
  ASSERT_THAT(dict->isRehashing(), true);
  dict->rehashMilliseconds(1000);
  ASSERT_THAT(dict->isRehashing(), false);
}

TEST(dict, replace) {
  int dur = 10;
  for (Var i = 0; std::get<int>(i) < kLen; std::get<int>(i) += dur) {
    Var val(std::get<int>(i) * 2);
    dict->replace(i, val);
  }
  for (Var i = 0; std::get<int>(i) < kLen; std::get<int>(i)++) {
    auto iter = dict->get(i);
    if (std::get<int>(i) % dur == 0) {
      ASSERT_THAT(std::get<int>(iter->key) * 2, std::get<int>(iter->value));
    } else {
      ASSERT_THAT(iter->key, iter->value);
    }
  }
}

TEST(dict, remove) {
  int dur = 4;
  for (int i = 0; i < kLen; i += dur) {
    dict->remove(i);
  }
  for (int i = 0; i < kLen; i++) {
    if (i % dur == 0) {
      ASSERT_THAT(dict->get(i), nullptr);
    } else {
      ASSERT_THAT(dict->get(i), Not(nullptr));
    }
  }
}

TEST(dict, scan) {
  rd::size_type cursor = 0, count = 0;
  do {
    cursor = dict->scan(
        cursor,
        [&count](rd::Dict::pointer entry) {
          count++;
        }
    );
  } while (cursor);
  ASSERT_THAT(count, dict->size());
}
