#ifndef PROVISIONING_MANAGER_HPP
#define PROVISIONING_MANAGER_HPP

#include "I_WiFiProv.hpp"
#include "util.hpp"
#include "wifi_aux.hpp"
#include <functional>
#include <memory>
#include <type_traits>

template <typename T>
concept I_Provisioning = requires(T t, const T ct) {
    { ct.is_provisioned() } -> std::same_as<bool>;
    { t.connect() } -> std::same_as<int>;
    { t.request_reprovision() } -> std::same_as<int>;
} && std::is_constructible_v<T, typename T::EventGroup &, std::string_view>;

template <typename T>
concept ProvisioningTypes =
    I_WiFiProv<typename T::WiFiProv> &&
    util::I_EventGroup<typename T::EventGroup> &&
    std::is_constructible_v<typename T::EventHandler, typename T::EventGroup &,
                            uint8_t>;

template <typename Types, typename T>
concept ProvHandlerFactory =
    ProvisioningTypes<Types> && util::Returns<T, typename Types::EventHandler,
                                              typename Types::EventGroup &>;

template <ProvisioningTypes T> class Provisioning {
    using WiFiProv = typename T::WiFiProv;
    using Handler = typename T::EventHandler;

  public:
    using EventGroup = typename T::EventGroup;

    Provisioning(EventGroup &event_group, std::string_view prov_name);
    bool is_provisioned() const;
    int connect();
    int request_reprovision();

  protected:
    void start();
    int hot_reset();
    int reset();

    WiFiProv wifi_prov{};
    EventGroup *event_group;
    Handler handler;
    std::string service_name;
    const WiFiProvSecurity security = WiFiProvSecurity::S1;
    const char pop{};
};

extern const char *PROV_TAG;

namespace provisioning {
void print_qr(const char *name, const char *transport);
std::string get_service_name();

extern uint8_t custom_service_uuid[16];
} // namespace provisioning

#include "Provisioning.tpp"

#endif
