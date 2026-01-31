#include "MockNVSHandle.hpp"
#include "config.hpp"
#include "nvs.hpp"
#include <gtest/gtest.h>

class ConfigTest : public testing::Test {
  protected:
    ConfigTest() { nvsstore = std::make_unique<MockNVS>(); }

    std::unique_ptr<MockNVS> nvsstore;
};

TEST_F(ConfigTest, init) {
    auto config = Config{*nvsstore};
    ASSERT_EQ(1, 1);
}
