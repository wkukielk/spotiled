#ifndef NVS_HPP
#define NVS_HPP

#include "I_NVSSecurity.hpp"
#include "esp_err.h"
#include "nvs_handle.hpp"
#include "util.hpp"
#include <ctime>
#include <vector>

extern const char *nvs_ns;

class I_NVS {
  public:
    using K = const char *;
    virtual ~I_NVS() noexcept {};
    virtual void write(K key, K value) = 0;
    virtual std::string read(const char *key) const = 0;
    virtual std::string read_or_throw(const char *key) const = 0;
    virtual std::string read(K key, std::string_view dft) const = 0;
    virtual void write_time_t(K key, time_t value) = 0;
    virtual int read_time_t(K key, time_t &value) const = 0;
};

class NVSUnencrypted : public I_NVSSecurity {
    bool is_flash_encrypted() const noexcept override { return false; };
    esp_err_t read_security_cfg(nvs_sec_cfg_t *cfg) override;
    esp_err_t secure_init_partition(const char *name,
                                    nvs_sec_cfg_t *cfg) override;
    esp_err_t generate_keys(nvs_sec_cfg_t *cfg) override;
};

class NVS : public I_NVS {
  public:
    explicit NVS(I_NVSSecurity &security);
    NVS(std::string_view partition_label, I_NVSSecurity &security);
    NVS(const NVS &) = delete;
    NVS(NVS &&);
    ~NVS() noexcept override;

    esp_err_t init();
    esp_err_t erase();
    void write(K key, const char *value) override;
    std::string read(K key) const override;
    std::string read_or_throw(K key) const override;
    std::string read(K key, std::string_view dft) const override;
    void write_time_t(K key, time_t value) override;
    int read_time_t(K key, time_t &value) const override;

  private:
    using NVSHandle = nvs::NVSHandle;
    u_ptr<nvs::NVSHandle> open(nvs_open_mode_t mode) const;
    esp_err_t get_string_size(NVSHandle &h, K key, size_t &size) const;
    esp_err_t get_string(NVSHandle &h, K key, char *out, size_t len) const;
    esp_err_t init_partition();

    std::string part{};
    std::reference_wrapper<I_NVSSecurity> security;
    static const char *ns;
};

#endif
