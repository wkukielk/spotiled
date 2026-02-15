#include "errors.hpp"
#include "nvs.hpp"
#include "nvs_handle.hpp"
#include "straux.hpp"
#include "util.hpp"
#include <gtest/gtest.h>

struct NVSPartitionLabel {
    const char *part_lbl;
};

class NVSTest : public testing::TestWithParam<NVSPartitionLabel> {
  protected:
    NVSTest() {
        const auto &p = GetParam();
        nvsstore = p.part_lbl ? mkuptr<NVS>(p.part_lbl) : mkuptr<NVS>();
    }
    ~NVSTest() { nvsstore->erase(); }

    std::unique_ptr<NVS> nvsstore;
};

const NVSPartitionLabel empty{};
const NVSPartitionLabel ns{};
INSTANTIATE_TEST_SUITE_P(NvsVariations, NVSTest, testing::Values(empty, ns));

TEST_P(NVSTest, write_read) {
    nvsstore->write_time_t("test", 42);
    time_t read = 0;
    ASSERT_EQ(nvsstore->read_time_t("test", std::ref(read)), 0);
    ASSERT_EQ(read, 42);
}

TEST_P(NVSTest, readNonExistent) {
    time_t read = 0;
    EXPECT_EQ(nvsstore->read_time_t("nonex", std::ref(read)),
              ESP_ERR_NVS_NOT_FOUND);
    EXPECT_EQ(read, 0);
}

TEST_P(NVSTest, writeNullKey) {
    ASSERT_THROW(nvsstore->write_time_t(nullptr, 42), NullError);
}

TEST_P(NVSTest, readNullKey) {
    time_t read{};
    ASSERT_THROW(nvsstore->read_time_t(nullptr, std::ref(read)), NullError);
}

TEST_P(NVSTest, erase) {
    nvsstore->write_time_t("test", 42);
    nvsstore->erase();
    time_t read = 0;
    EXPECT_EQ(nvsstore->read_time_t("test", std::ref(read)),
              ESP_ERR_NVS_NOT_FOUND);
}

TEST_P(NVSTest, str) {
    nvsstore->write("test", "VALUE");
    auto read = nvsstore->read("test");
    ASSERT_STREQ(read.data(), "VALUE");
    ASSERT_EQ(read.size(), 5);
}

TEST_P(NVSTest, writeEmptyStr) {
    nvsstore->write("test", "");
    auto read = nvsstore->read("test");
    ASSERT_STREQ(read.data(), "");
}

TEST_P(NVSTest, writeNull) {
    ASSERT_THROW(nvsstore->write("test", nullptr), NullError);
}

TEST_P(NVSTest, writeNullKeyStr) {
    ASSERT_THROW(nvsstore->write(nullptr, "VALUE"), NullError);
}

TEST_P(NVSTest, readNullKeyStr) {
    ASSERT_THROW(nvsstore->read(nullptr), NullError);
}

TEST(NVSTestTwoPartitions, write_read) {
    auto nvsstore1 = mkuptr<NVS>();
    auto nvsstore2 = mkuptr<NVS>("cfg");
    nvsstore1->write_time_t("test", 10);
    nvsstore2->write_time_t("test", 20);
    time_t read1 = 0;
    time_t read2 = 0;
    ASSERT_EQ(nvsstore1->read_time_t("test", std::ref(read1)), 0);
    ASSERT_EQ(nvsstore2->read_time_t("test", std::ref(read2)), 0);
    ASSERT_EQ(read1, 10);
    ASSERT_EQ(read2, 20);
}