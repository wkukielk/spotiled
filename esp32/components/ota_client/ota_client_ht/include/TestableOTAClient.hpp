#ifndef TESTABLE_OTA_CLIENT_HPP
#define TESTABLE_OTA_CLIENT_HPP

#include "MockOTA.hpp"
#include "OTAClient.hpp"

class TestableOTAClient : public OTAClient<MockOTA> {
  public:
    using OTAClient<MockOTA>::OTAClient;
    MockOTA &get_ota() { return ota; }
};

#endif
