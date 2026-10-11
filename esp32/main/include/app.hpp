#ifndef APP_HPP
#define APP_HPP

#include "Configurer.hpp"
#include "HTTPD.hpp"
#include "Handle.hpp"
#include "LED.hpp"
#include "NVSSecurity.hpp"
#include "UDPServer.hpp"
#include "UDPSocket.hpp"
#include "WSServer.hpp"
#include "animator.hpp"
#include "global.hpp"
#include "nvs.hpp"
#include "runtime.hpp"
#include "server/led_color_handler.hpp"
#include "server/udp_thread.hpp"
#include "util.hpp"
#include <functional>

using MyLEDC = LEDC<LED>;
using MyAnimator = Animator<MyLEDC>;

struct RuntimeServerThreadTypes {
    using WSServer = ::WSServer<HTTPD>;
    using Queue = util::PointerQueue<server::Data>;
    using Handler = CommandHandler<MyAnimator>;
};

struct RuntimeUDPThreadTypes {
    using UDPServer = ::UDPServer<UDPSocket>;
    using Handler = LedColorHandler<MyAnimator>;
};

struct RuntimeTypes {
    using EGroup = util::EventGroup;
    using T_OTAThread = OTAThread<OTAClient<OTA>>;
    using T_ServerThread = ServerThread<RuntimeServerThreadTypes>;
    using T_UDPThread = UDPThread<RuntimeUDPThreadTypes>;
    using T_LEDC = MyAnimator;
};

using runtime_t = Runtime<RuntimeTypes>;

struct AppHandles {
    Handle<DftEventLoop> eloop;
    Netif netif;
    u_ptr<NVSSecurity> nvs_security;
    u_ptr<NVSUnencrypted> nvs_plain;
    NVSHandle nvs;
    TimeHandle time;
    MDNSHandle mdns;
    Handle<LED> led;
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
