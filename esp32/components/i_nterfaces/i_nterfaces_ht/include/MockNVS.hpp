#ifndef MOCK_NVS_HPP
#define MOCK_NVS_HPP

#include "nvs.hpp"
#include <gmock/gmock.h>

class MockNVS : public I_NVS {
    using K = const char *;

  public:
    MOCK_METHOD(void, write, (K key, K value), (override));
    MOCK_METHOD(std::string, read, (const char *key), (const, override));
    MOCK_METHOD(std::string, read_or_throw, (const char *key),
                (const, override));
    MOCK_METHOD(std::string, read, (K key, std::string_view dft),
                (const, override));
    MOCK_METHOD(void, write_time_t, (K key, time_t value), (override));
    MOCK_METHOD(int, read_time_t, (K key, time_t &value), (const, override));
};

#endif
