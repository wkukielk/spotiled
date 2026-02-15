#ifndef OTA_HPP
#define OTA_HPP

#include <string_view>

struct ota_config {
    std::string_view url;
};

class OTA {
  public:
    OTA();
};

#endif
