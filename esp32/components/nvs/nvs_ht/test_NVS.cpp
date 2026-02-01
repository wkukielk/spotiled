#include "nvs.hpp"
#include "nvs_handle.hpp"
#include "straux.hpp"
#include <gtest/gtest.h>

class NVSTest : public testing::Test {
  protected:
    NVSTest() {
        nvsstore = std::make_unique<NVS>();
        nvsstore->erase();
    }
    ~NVSTest() { nvsstore->erase(); }

    std::unique_ptr<NVS> nvsstore;
};

TEST_F(NVSTest, write_read) {
    nvsstore->write_time_t("test", 42);
    time_t read = 0;
    ASSERT_EQ(nvsstore->read_time_t("test", std::ref(read)), 0);
    ASSERT_EQ(read, 42);
}

TEST_F(NVSTest, readNonExistent) {
    time_t read = 0;
    EXPECT_EQ(nvsstore->read_time_t("nonex", std::ref(read)),
              ESP_ERR_NVS_NOT_FOUND);
    EXPECT_EQ(read, 0);
}

TEST_F(NVSTest, erase) {
    nvsstore->write_time_t("test", 42);
    nvsstore->erase();
    time_t read = 0;
    EXPECT_EQ(nvsstore->read_time_t("test", std::ref(read)),
              ESP_ERR_NVS_NOT_FOUND);
}

TEST_F(NVSTest, str) {
    nvsstore->write("test", "VALUE");
    auto read = straux::cvector<char>{};
    ASSERT_EQ(nvsstore->read("test", std::ref(read)), 0);
    ASSERT_STREQ(read.data(), "VALUE");
}
