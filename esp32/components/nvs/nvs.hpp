#ifndef NVS_HPP
#define NVS_HPP

#include "esp_err.h"
#include "straux.hpp"
#include <ctime>
#include <vector>

extern const char *nvs_ns;

class I_NVS {
  public:
    using K = const char *;
    virtual ~I_NVS() noexcept {};
    virtual void write(K key, K value) = 0;
    virtual int read(K key, CharBuf &buf) const = 0;
    virtual CharBuf read(K key, std::string_view dft) const = 0;
    virtual void write_time_t(K key, time_t value) = 0;
    virtual int read_time_t(K key, time_t &value) const = 0;
};

class NVS : public I_NVS {
  public:
    NVS();
    explicit NVS(const char *partition_label);
    ~NVS();

    esp_err_t erase();
    void write(const char *key, const char *value) override;
    int read(const char *key, CharBuf &buf) const override;
    CharBuf read(K key, std::string_view dft) const override;
    void write_time_t(const char *key, time_t value) override;
    int read_time_t(const char *key, time_t &value) const override;
};

#endif
