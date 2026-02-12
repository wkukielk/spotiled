#ifndef NVS_HPP
#define NVS_HPP

#include "esp_err.h"
#include "nvs_handle.hpp"
#include "straux.hpp"
#include "util.hpp"
#include <ctime>
#include <vector>

extern const char *nvs_ns;

class I_NVS {
  public:
    using K = const char *;
    virtual ~I_NVS() noexcept {};
    virtual void write(K key, K value) = 0;
    virtual CharBuf read(const char *key) const = 0;
    virtual CharBuf read_or_throw(const char *key) const = 0;
    virtual CharBuf read(K key, std::string_view dft) const = 0;
    virtual void write_time_t(K key, time_t value) = 0;
    virtual int read_time_t(K key, time_t &value) const = 0;
};

class NVS : public I_NVS {
  public:
    NVS();
    explicit NVS(std::string_view partition_label);
    ~NVS() noexcept override;

    esp_err_t erase();
    void write(K key, const char *value) override;
    CharBuf read(K key) const override;
    CharBuf read_or_throw(K key) const override;
    CharBuf read(K key, std::string_view dft) const override;
    void write_time_t(K key, time_t value) override;
    int read_time_t(K key, time_t &value) const override;

  private:
    using NVSHandle = nvs::NVSHandle;
    u_ptr<nvs::NVSHandle> open(nvs_open_mode_t mode) const;
    esp_err_t get_string_size(NVSHandle &h, K key, size_t &size) const;
    esp_err_t get_string(NVSHandle &h, K key, char *out, size_t len) const;
    std::string part{};
    static const char *ns;
};

#endif
