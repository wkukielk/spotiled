#include "MockMDNS.hpp"
#include "MockNVS.hpp"
#include "PersistentMDNS.hpp"
#include "errors.hpp"
#include <gtest/gtest.h>

using namespace testing;

class PersistentMDNSTest : public Test {
  protected:
    MockNVS nvs{};
    PersistentMDNS<MockMDNS> mdns{nvs, "MDNS_INSTANCE"};
};

TEST_F(PersistentMDNSTest, SetInstanceNamePersistsThenDelegates) {
    InSequence seq;
    EXPECT_CALL(nvs, write(StrEq("MDNS_INSTANCE"), StrEq("newname")));
    EXPECT_CALL(mdns, set_instance_name(StrEq("newname")))
        .WillOnce(Return(ESP_OK));

    EXPECT_EQ(mdns.set_instance_name("newname"), ESP_OK);
}

TEST_F(PersistentMDNSTest, SetInstanceNameReturnsInnerFailure) {
    EXPECT_CALL(nvs, write(_, _));
    EXPECT_CALL(mdns, set_instance_name(_)).WillOnce(Return(ESP_FAIL));

    EXPECT_EQ(mdns.set_instance_name("newname"), ESP_FAIL);
}

TEST_F(PersistentMDNSTest, SetInstanceNameSkipsInnerWhenPersistFails) {
    EXPECT_CALL(nvs, write(_, _)).WillOnce(Throw(NVSError(ESP_FAIL, "write")));
    EXPECT_CALL(mdns, set_instance_name(_)).Times(0);

    EXPECT_EQ(mdns.set_instance_name("newname"), ESP_FAIL);
}

TEST_F(PersistentMDNSTest, OtherMethodsForwardUnchanged) {
    EXPECT_CALL(nvs, write(_, _)).Times(0);
    EXPECT_CALL(mdns, set_hostname(StrEq("host"))).WillOnce(Return(ESP_OK));

    EXPECT_EQ(mdns.set_hostname("host"), ESP_OK);
}
