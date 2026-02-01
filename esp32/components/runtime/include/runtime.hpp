#include "WiFiManager.hpp"

class Runtime {
  public:
    Runtime(u_ptr<I_WiFiManager> wifi_mgr);
    void run();

  private:
    void setup();
    u_ptr<I_WiFiManager> wifi_mgr{};
};
