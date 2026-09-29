#include "MockNVSSecurity.hpp"
#include "errors.hpp"
#include "nvs.hpp"
#include "nvs_handle.hpp"
#include "straux.hpp"
#include "util.hpp"
#include <gtest/gtest.h>

using namespace testing;

u_ptr<NVS> create_unencrypted_nvs(I_NVSSecurity &sec, const char *part_lbl) {
    return part_lbl ? mkuptr<NVS>(part_lbl, sec) : mkuptr<NVS>(sec);
}

struct NVSPartitionLabel {
    const char *part_lbl;
};

class NVSTest : public testing::TestWithParam<NVSPartitionLabel> {
  protected:
    NVSTest() {
        const auto &p = GetParam();
        auto sec = mkuptr<NVSUnencrypted>();
        nvsstore = create_unencrypted_nvs(*sec, p.part_lbl);
        nvsstore->init();
    }
    ~NVSTest() { nvsstore->erase(); }

    std::unique_ptr<NVS> nvsstore;
};

const NVSPartitionLabel empty{};
const NVSPartitionLabel ns{"cfg"};
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

class NVSTestInit : public testing::Test {
  protected:
    NVSTestInit() : security{}, nvsstore{security} {}

    MockNVSSecurity security;
    NVS nvsstore;
};

TEST_F(NVSTestInit, unencrypted) {
    EXPECT_CALL(security, is_flash_encrypted()).WillOnce(Return(false));
    nvsstore.init();
}

TEST_F(NVSTestInit, encryptedFirstBoot) {
    InSequence inseq;

    EXPECT_CALL(security, is_flash_encrypted()).WillOnce(Return(true));
    EXPECT_CALL(security, read_security_cfg(_))
        .WillOnce(Return(ESP_ERR_NVS_KEYS_NOT_INITIALIZED));
    EXPECT_CALL(security, generate_keys(_)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(security,
                secure_init_partition(StrEq(NVS_DEFAULT_PART_NAME), _))
        .WillOnce(Return(ESP_OK));

    nvsstore.init();
}

TEST_F(NVSTestInit, encryptedNextBoot) {
    InSequence inseq;

    EXPECT_CALL(security, is_flash_encrypted()).WillOnce(Return(true));
    EXPECT_CALL(security, read_security_cfg(_)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(security,
                secure_init_partition(StrEq(NVS_DEFAULT_PART_NAME), _))
        .WillOnce(Return(ESP_OK));

    nvsstore.init();
}

TEST(NVSTestTwoPartitions, write_read) {
    auto sec = mkuptr<NVSUnencrypted>();
    auto nvsstore1 = create_unencrypted_nvs(*sec, nullptr);
    nvsstore1->init();
    auto nvsstore2 = create_unencrypted_nvs(*sec, "cfg");
    nvsstore2->init();

    nvsstore1->write_time_t("test", 10);
    nvsstore2->write_time_t("test", 20);
    time_t read1 = 0;
    time_t read2 = 0;
    ASSERT_EQ(nvsstore1->read_time_t("test", std::ref(read1)), 0);
    ASSERT_EQ(nvsstore2->read_time_t("test", std::ref(read2)), 0);
    ASSERT_EQ(read1, 10);
    ASSERT_EQ(read2, 20);
}