#include "straux.hpp"
#include <gtest/gtest.h>

class StrauxTest : public testing::Test {
  protected:
    StrauxTest() {}
};

TEST_F(StrauxTest, strprintf) {
    auto str = straux::strprintf("%s", "hello");
    ASSERT_EQ(str, "hello");
}

TEST_F(StrauxTest, badFormat) {
    EXPECT_NO_THROW(straux::strprintf("%d", "hello"));
}

TEST_F(StrauxTest, strapp) {
    auto *str1 = new char[20];
    strcpy(str1, "hello");
    straux::strapp(str1 + 5, " world");
    ASSERT_STREQ(str1, "hello world");
}

TEST_F(StrauxTest, strappEmpty) {
    auto *str1 = new char[20];
    strcpy(str1, "hello");
    straux::strapp(str1 + 3, "");
    ASSERT_STREQ(str1, "hel");
}

TEST_F(StrauxTest, strcopy) {
    const char *str = "hello";
    auto copy = std::unique_ptr<char>{straux::strcopy(str)};
    ASSERT_STREQ(str, copy.get());
}

TEST_F(StrauxTest, strcopyEmpty) {
    const char *str = "";
    auto copy = std::unique_ptr<char>{straux::strcopy(str)};
    ASSERT_STREQ(str, copy.get());
}
