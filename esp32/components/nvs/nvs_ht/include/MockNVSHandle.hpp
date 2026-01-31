#ifndef MOCK_NVS_HHANDLE_HPP
#define MOCK_NVS_HHANDLE_HPP

#include "nvs.hpp"
#include <gmock/gmock.h>

class MockNVS : public I_NVS {
    using BufT = straux::cvector<char>;

  public:
    MOCK_METHOD(void, write, (const char *key, const char *value), (override));
    MOCK_METHOD(int, read, (const char *key, BufT &buf), (override));
    MOCK_METHOD(void, write_time_t, (const char *key, time_t value),
                (override));
    MOCK_METHOD(int, read_time_t, (const char *key, time_t &value), (override));
};

#endif
