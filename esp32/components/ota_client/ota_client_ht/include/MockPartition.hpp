#ifndef MOCK_PARTITION_HPP
#define MOCK_PARTITION_HPP

#include "I_OTA.hpp"
#include <gmock/gmock.h>

class MockPartition : public I_Partition {
  public:
    MOCK_METHOD(esp_err_t, get_state, (esp_ota_img_states_t * ota_state),
                (const, noexcept, override));
    MOCK_METHOD(esp_err_t, get_description, (esp_app_desc_t * app_desc),
                (const, noexcept, override));
};

#endif
