#include "nvs.hpp"
#include "nvs_handle.hpp"
#include <gtest/gtest.h>

class NVSTest : public testing::Test {
  protected:
    NVSTest() { nvsstore = std::make_unique<NVS>(); }
    ~NVSTest() { nvsstore->erase(); }

    std::unique_ptr<NVS> nvsstore;
};

TEST_F(NVSTest, write_read) {
    nvsstore->erase();
    nvsstore->write_time_t("test", 42);
    time_t read = 0;
    ASSERT_EQ(nvsstore->read_time_t("test", std::ref(read)), 0);
    ASSERT_EQ(read, 42);
}

TEST_F(NVSTest, readNonExistent) {
    nvsstore->erase();
    time_t read = 0;
    EXPECT_EQ(nvsstore->read_time_t("nonex", std::ref(read)),
              ESP_ERR_NVS_NOT_FOUND);
    EXPECT_EQ(read, 0);
}

TEST_F(NVSTest, erase) {
    nvsstore->erase();
    nvsstore->write_time_t("test", 42);
    nvsstore->erase();
    time_t read = 0;
    EXPECT_EQ(nvsstore->read_time_t("test", std::ref(read)),
              ESP_ERR_NVS_NOT_FOUND);
}
