#ifndef APP_HPP
#define APP_HPP

#include "Configurer.hpp"
#include "nvs.hpp"
#include "runtime.hpp"
#include "util.hpp"

class App {
  public:
    void run();
    static App *create();

  private:
    App(u_ptr<Runtime> &&runtime);

    u_ptr<Runtime> runtime{};
};

#endif
