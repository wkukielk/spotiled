#ifndef APP_HPP
#define APP_HPP

#include "nvs.hpp"
#include "runtime.hpp"
#include "util.hpp"

class App {
  public:
    App();
    void run();

  private:
    u_ptr<NVS> cfgnvs{};
    u_ptr<Runtime> runtime{};
};

#endif
