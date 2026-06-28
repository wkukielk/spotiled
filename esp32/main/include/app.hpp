#ifndef APP_HPP
#define APP_HPP

#include "Configurer.hpp"
#include "HTTPD.hpp"
#include "Handle.hpp"
#include "LED.hpp"
#include "global.hpp"
#include "nvs.hpp"
#include "runtime.hpp"
#include "util.hpp"
#include <functional>

struct RuntimeServerThreadTypes {
    using MyLEDC = LEDC<LED, Handle<DftEventLoop>>;

    using WSServer = ::WSServer<HTTPD>;
    using Queue = util::PointerQueue<server::Data>;
    using Handler = CommandHandler<MyLEDC>;
};

struct RuntimeTypes {
    using EGroup = util::EventGroup;
    using T_OTAThread = OTAThread<OTAClient<OTA>>;
    using T_ServerThread = ServerThread<RuntimeServerThreadTypes>;
};

using runtime_t = Runtime<RuntimeTypes>;

struct AppHandles {
    Handle<DftEventLoop> eloop;
    Netif netif;
    NVSHandle nvs;
    TimeHandle time;
    MDNSHandle mdns;
    u_ptr<runtime_t> runtime;
};

class App {
  public:
    void run();
    static App create();

  private:
    App(AppHandles &&handles);

    AppHandles handles;
};

#endif
