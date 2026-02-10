#include "CharVector.hpp"
#include <gtest/gtest.h>

class CharVectorTest : public testing::Test {
  protected:
    CharVectorTest() {}

    CharVector v{};
    CharBuf out{};
    const char data[9] = "poziomka";
};

TEST_F(CharVectorTest, init) { ASSERT_FALSE(v.is_setup()); }

TEST_F(CharVectorTest, emptyRelease) {
    v.release(out);
    ASSERT_TRUE(out.empty());
}

TEST_F(CharVectorTest, emptyExtend) {
    ASSERT_EQ(v.extend(data, 9), -1);
    v.release(out);
    ASSERT_TRUE(out.empty());
}

TEST_F(CharVectorTest, emptyPush) {
    ASSERT_EQ(v.push('c'), -1);
    v.release(out);
    ASSERT_TRUE(out.empty());
}

TEST_F(CharVectorTest, setup) {
    v.setup(5);
    ASSERT_TRUE(v.is_setup());
    v.release(out);
    ASSERT_EQ(out.size(), 5);
}

TEST_F(CharVectorTest, setup0) {
    v.setup(0);
    ASSERT_FALSE(v.is_setup());
}

TEST_F(CharVectorTest, setup1) {
    v.setup(1);
    ASSERT_TRUE(v.is_setup());
    v.release(out);
    ASSERT_STREQ(out.data(), "");
}

TEST_F(CharVectorTest, extend) {
    v.setup(10);
    ASSERT_EQ(v.extend(data, 9), 9);
    v.release(out);
    ASSERT_STREQ(out.data(), data);
}

TEST_F(CharVectorTest, doubleExtend) {
    v.setup(20);
    v.extend(data, 8);
    v.extend(data, 9);
    v.release(out);
    ASSERT_STREQ(out.data(), "poziomkapoziomka");
}

TEST_F(CharVectorTest, extendPush) {
    v.setup(10);
    v.extend(data, 8);
    v.push('1');
    v.push('\0');
    v.release(out);
    ASSERT_STREQ(out.data(), "poziomka1");
}

TEST_F(CharVectorTest, push) {
    v.setup(10);
    ASSERT_EQ(v.push('1'), 0);
    v.push('\0');
    v.release(out);
    ASSERT_STREQ(out.data(), "1");
}

TEST_F(CharVectorTest, noSpaceForExtend) {
    v.setup(10);
    v.extend(data, 8);
    ASSERT_EQ(v.extend(data, 9), 1);
    v.release(out);
    ASSERT_STREQ(out.data(), "poziomkap");
}

TEST_F(CharVectorTest, justRightExtend) {
    v.setup(9);
    ASSERT_EQ(v.extend(data, 8), 8);
    ASSERT_EQ(v.push('\0'), -1);
    v.release(out);
    ASSERT_STREQ(out.data(), "poziomka");
}

TEST_F(CharVectorTest, extendOneTooMany) {
    v.setup(8);
    ASSERT_EQ(v.extend(data, 8), 7);
    v.release(out);
    ASSERT_STREQ(out.data(), "poziomk");
}

TEST_F(CharVectorTest, clear) {
    v.setup(10);
    v.extend(data, 8);
    v.clear();
    EXPECT_FALSE(v.is_setup());
    v.release(out);
    EXPECT_EQ(out.size(), 0);
}