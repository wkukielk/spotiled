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
    App(u_ptr<Runnable> &&runtime);

    u_ptr<Runnable> runtime;
};

#endif
