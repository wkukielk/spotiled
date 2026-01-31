#ifndef NVS_HPP
#define NVS_HPP

#include "esp_err.h"
#include "straux.hpp"
#include <ctime>
#include <vector>

extern const char *nvs_ns;

class I_NVS {
  public:
    virtual ~I_NVS() noexcept {};
    virtual void write(const char *key, const char *value) = 0;
    virtual int read(const char *key, straux::cvector<char> &buf) = 0;
    virtual void write_time_t(const char *key, time_t value) = 0;
    virtual int read_time_t(const char *key, time_t &value) = 0;
};

class NVS : public I_NVS {
  public:
    NVS();
    explicit NVS(const char *partition_label);
    ~NVS();

    esp_err_t erase();
    void write(const char *key, const char *value) override;
    int read(const char *key, straux::cvector<char> &buf) override;
    void write_time_t(const char *key, time_t value) override;
    int read_time_t(const char *key, time_t &value) override;
};

#endif
