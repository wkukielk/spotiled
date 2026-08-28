#ifndef WIFI_EVENT_HANDLER_HPP
#define WIFI_EVENT_HANDLER_HPP

#include "I_DftEventLoop.hpp"
#include "I_WiFi.hpp"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "util.hpp"
#include "wifi_aux.hpp"

template <typename T>
concept WiFiEventHandlerTypes = WiFiHandle<typename T::WiFi> &&
                                util::I_EventGroup<typename T::EventGroup> &&
                                DftEventLoopHandle<typename T::EventLoop>;

// only one per program
template <WiFiEventHandlerTypes T> class WiFiEventHandler {
    using WiFi = typename T::WiFi;
    using EGroup = typename T::EventGroup;
    using DEL = typename T::EventLoop;

  public:
    WiFiEventHandler(EGroup &event_group, uint8_t retries);
    ~WiFiEventHandler() noexcept;

    WiFiEventHandler(const WiFiEventHandler &) = delete;
    WiFiEventHandler &operator=(const WiFiEventHandler &) = delete;
    WiFiEventHandler(WiFiEventHandler &&other) noexcept;

  private:
    esp_err_t register_self() noexcept;
    static void handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);
    static void ip_handler(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data);

    static EGroup *event_group;
    static uint8_t max_retries;
    static int8_t retries_left;

    esp_event_handler_instance_t instance{};
    esp_event_handler_instance_t ip_instance{};
};

#include "WiFiEventHandler.tpp"

#endif
