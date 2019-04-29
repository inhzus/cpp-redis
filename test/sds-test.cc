//
// Created by suun on 9/30/2018.
//

#include "redis.h"
#include <gmock/gmock.h>

const char *ptr = "what";
const char *secondPtr = "ip";
const char *thirdPtr = "what the fuck";

const char *firstAndSecondPtr = "whatip";

TEST(sds, sdsnew) {
  rd::String s1(ptr);
  ASSERT_STREQ(s1.begin(), ptr);
  testing::internal::CaptureStdout();
  std::cout << s1;
  ASSERT_STREQ(ptr, testing::internal::GetCapturedStdout().c_str());
}

TEST(sds, sdsempty) {
  rd::String s2;
  ASSERT_STREQ(s2.begin(), nullptr);
  testing::internal::CaptureStdout();
  std::cout << s2;
  ASSERT_STREQ("", testing::internal::GetCapturedStdout().c_str());
}

TEST(sds, sdsfree) {
  rd::String s1(ptr);
  s1.~String();
  ASSERT_STREQ(s1.begin(), nullptr);
  testing::internal::CaptureStdout();
  std::cout << s1;
  ASSERT_STREQ("", testing::internal::GetCapturedStdout().c_str());
}

TEST(sds, sdslen) {
  rd::String s1(ptr);
  ASSERT_EQ(s1.size(), 4);
  s1.~String();
  ASSERT_EQ(s1.size(), 0);
}

TEST(sds, sdsavail) {
  rd::String s1(ptr);
  ASSERT_EQ(s1.capacity(), 4);
  s1 += secondPtr;
  ASSERT_EQ(s1.capacity(), 8);
  s1 += thirdPtr;
  ASSERT_EQ(s1.capacity(), 21);
}

TEST(sds, sdsdmp) {
  rd::String s1(ptr), s2(secondPtr), s3(thirdPtr);
  s1 = s2;
  ASSERT_EQ(s1.capacity(), 4);
  ASSERT_EQ(s1.size(), 2);
  ASSERT_STREQ(s1.begin(), secondPtr);

  s1 = s3;
  ASSERT_EQ(s1.size(), 13);
  ASSERT_STREQ(s1.begin(), thirdPtr);
}

TEST(sds, sdscat) {
  rd::String s1(ptr);
  s1 += secondPtr;
  ASSERT_STREQ(s1.begin(), firstAndSecondPtr);
}

TEST(sds, sdscatsds) {
  rd::String s1(ptr), s2(secondPtr);
  s1 += s2;
  ASSERT_STREQ(s1.begin(), firstAndSecondPtr);
}

TEST(sds, sdscpy) {
  rd::String s1(ptr);
  s1 = secondPtr;
  ASSERT_STREQ(s1.begin(), secondPtr);
}

TEST(sds, sdsgrowzero) {
  rd::String s1(ptr);
  s1.resize(10);
  ASSERT_STREQ(s1.begin(), ptr);
  ASSERT_EQ(s1.capacity(), 10);
}

TEST(sds, sdsrange) {
  rd::String s1(firstAndSecondPtr);
  rd::String s2 = s1.substr(0, 4);
  ASSERT_STREQ(ptr, s2.begin());
  rd::String s3 = s1.substr(4, 6);
  ASSERT_STREQ(s3.begin(), secondPtr);
}

TEST(sds, sdstrim) {
  rd::String s1("AA...AA.a.aa.awhat     :::");
  s1.trim("Aa. :");
  ASSERT_STREQ(s1.begin(), ptr);
}

TEST(sds, sdscmp) {
  rd::String s1(ptr), s2(ptr), s3(secondPtr);
  ASSERT_TRUE(s1 == s2);
  ASSERT_TRUE(s1 != s3);
}