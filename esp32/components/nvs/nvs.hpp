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
    virtual int read(K key, CharBuf &buf) const = 0;
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
    void write(const char *key, const char *value) override;
    int read(const char *key, CharBuf &buf) const override;
    CharBuf read(K key, std::string_view dft) const override;
    void write_time_t(const char *key, time_t value) override;
    int read_time_t(const char *key, time_t &value) const override;

  private:
    u_ptr<nvs::NVSHandle> open(nvs_open_mode_t mode, esp_err_t *err) const;
    std::string part{};
    static const char *ns;
};

#endif
