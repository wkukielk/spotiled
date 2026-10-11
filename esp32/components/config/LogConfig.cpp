#include "LogConfig.hpp"
#include "data.hpp"
#include "esp_log.h"
#include <algorithm>
#include <cstring>

namespace {
const char *const TAG = "LogConfig";
const char *hierarchy[] = {LOG::NONE, LOG::ERROR, LOG::WARN,
                           LOG::INFO, LOG::DEBUG, LOG::VERBOSE};
inline esp_log_level_t log_level(const char *level) noexcept {
    auto p = std::find_if(hierarchy, std::end(hierarchy),
                          [=](auto p) { return strcmp(level, p) == 0; });
    return (esp_log_level_t)(p - hierarchy);
};
}; // namespace

LogConfig::LogConfig(I_NVS &cfgnvs) : cfgnvs{&cfgnvs} {}

void LogConfig::configure() const {
    auto level_str = cfgnvs->read(key::LOG_LEVEL);
    auto level = log_level(level_str.data());
    if (level >= ESP_LOG_MAX)
        UTIL_LOGW("invalid log level config");
    else if (level > CONFIG_LOG_MAXIMUM_LEVEL)
        UTIL_LOGW("config log level > max level");
    else {
        esp_log_set_level_master(level);
        esp_log_level_set("*", level);
    }
}
