#ifndef RUNTIME_HELPERS_HPP
#define RUNTIME_HELPERS_HPP

#include "OTAClient.hpp"
#include "URL.hpp"
#include "runtime_data.hpp"
#include "server/server.hpp"
#include "util.hpp"
#include <expected>
#include <optional>

extern const char *runtime_tag;
extern const char *server_tag;
extern const EventBits_t OTA_BIT;
extern const EventBits_t SERVER_BIT;
extern const EventBits_t UDP_BIT;

template <typename T_OTAClient> class OTAThread : public util::PostponedThread {
  public:
    using Client = T_OTAClient;

    OTAThread(T_OTAClient &&ota_client, const util::JoinEventGroup &join_group);
    void start();

  private:
    static void ota_thread_fun(void *ota_thread_ptr);

    T_OTAClient ota_client;
};

std::expected<URL, esp_err_t> resolve_base(const URL &base_url);

namespace master {
class DataPrinter {
  public:
    const char *operator()(const master::ConnectionErrorData &) const noexcept {
        return "connection error";
    }
    const char *operator()(const master::ErrorData &) const noexcept {
        return "error";
    }
    const char *
    operator()(const master::ReprovisionRequest &) const noexcept {
        return "reprovision request";
    }
    const char *operator()(const master::ConnectRequest &) const noexcept {
        return "connect request";
    }
    const char *operator()(const master::StartRequest &) const noexcept {
        return "start request";
    }
    const char *operator()(const master::RestartRequest &) const noexcept {
        return "restart request";
    }
};
} // namespace master

#include "runtime_helpers.tpp"

#endif
