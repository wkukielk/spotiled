#ifndef MOCK_OTA_HPP
#define MOCK_OTA_HPP

#include "I_OTA.hpp"
#include <gmock/gmock.h>

class MockOTA : public I_FullOTA {
  public:
    MOCK_METHOD(esp_err_t, mark_app_valid, (), (noexcept));
    MOCK_METHOD(std::unique_ptr<I_Partition>, get_running_partition, (),
                (const, noexcept));
    MOCK_METHOD(std::unique_ptr<I_Partition>, get_other_partition, (),
                (const, noexcept));

    MOCK_METHOD(esp_err_t, update, (const ota::config *ota_config),
                (noexcept, override));
    MOCK_METHOD(esp_err_t, begin, (const ota::config *ota_config),
                (noexcept, override));
    MOCK_METHOD(esp_err_t, get_img_desc, (esp_app_desc_t * new_app_info),
                (const, noexcept, override));
    MOCK_METHOD(int, get_image_len_read, (), (const, noexcept, override));
    MOCK_METHOD(int, get_status_code, (), (const, noexcept, override));
    MOCK_METHOD(esp_err_t, perform, (), (noexcept, override));
    MOCK_METHOD(bool, is_all_received, (), (const, noexcept, override));
    MOCK_METHOD(esp_err_t, finish, (), (noexcept, override));
};

#endif
