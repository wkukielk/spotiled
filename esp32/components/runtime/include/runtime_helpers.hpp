#ifndef RUNTIME_HELPERS_HPP
#define RUNTIME_HELPERS_HPP

#include "OTAClient.hpp"
#include "URL.hpp"
#include "runtime_data.hpp"
#include "util.hpp"
#include <optional>

extern const char *runtime_tag;
extern const EventBits_t OTA_BIT;

struct OTAEventBits {
    std::reference_wrapper<util::I_EventGroup> egroup;
    EventBits_t join_bit;
};

class OTAThread {
  public:
    OTAThread(OTAClient &&ota_client, const OTAEventBits &event_bits);
    void start();
    void join() { thread->join(); };

  private:
    OTAClient ota_client;
    OTAEventBits event_bits;
    std::optional<util::Thread> thread;
};

URL resolve_base(const URL &base_url);

class DataPrinter {
  public:
    const char *operator()(const master::ConnectionErrorData &) const noexcept {
        return "connection error";
    }
    const char *operator()(const master::ErrorData &) const noexcept {
        return "error";
    }
};

#endif
