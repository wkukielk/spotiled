#ifndef RUNTIME_HELPERS_HPP
#define RUNTIME_HELPERS_HPP

#include "OTAClient.hpp"
#include "URL.hpp"
#include "runtime_data.hpp"
#include "server/server.hpp"
#include "util.hpp"
#include <optional>

extern const char *runtime_tag;
extern const char *server_tag;
extern const EventBits_t OTA_BIT;

struct JoinEventGroup {
    std::reference_wrapper<util::I_EventGroup> egroup;
    EventBits_t join_bit;
};

class PostponedThread {
  public:
    PostponedThread(const JoinEventGroup &join_group);
    void join() { thread->join(); };

  protected:
    JoinEventGroup join_group;
    std::optional<util::Thread> thread;
};

class OTAThread : public PostponedThread {
  public:
    OTAThread(OTAClient &&ota_client, const JoinEventGroup &join_group);
    void start();

  private:
    OTAClient ota_client;
};

template <typename HTTPD> class ServerThread : public PostponedThread {
  public:
    ServerThread(Server<HTTPD> &&server, const JoinEventGroup &join_group);
    void start();

  private:
    Server<HTTPD> server;
};

URL resolve_base(const URL &base_url);

namespace master {
class DataPrinter {
  public:
    const char *operator()(const master::ConnectionErrorData &) const noexcept {
        return "connection error";
    }
    const char *operator()(const master::ErrorData &) const noexcept {
        return "error";
    }
};
} // namespace master

#include "runtime_helpers.tpp"

#endif
