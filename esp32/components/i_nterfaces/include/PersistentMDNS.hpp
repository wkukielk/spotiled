#ifndef PERSISTENT_MDNS_HPP
#define PERSISTENT_MDNS_HPP

#include "I_mDNSFacade.hpp"
#include "esp_log.h"
#include "nvs.hpp"
#include <concepts>
#include <exception>
#include <utility>

template <typename T>
    requires std::derived_from<T, I_mDNSFacade>
class PersistentMDNS : public T {
    using K = const char *;

  public:
    template <typename... Args>
    PersistentMDNS(I_NVS &nvs, K key, Args &&...args)
        : T{std::forward<Args>(args)...}, nvs{&nvs}, key{key} {}

    esp_err_t set_instance_name(K instance) override {
        try {
            nvs->write(key, instance);
        } catch (const std::exception &e) {
            ESP_LOGE("PersistentMDNS", "Failed to persist instance name: %s",
                     e.what());
            return ESP_FAIL;
        }
        return T::set_instance_name(instance);
    }

  private:
    I_NVS *nvs;
    K key;
};

#endif
