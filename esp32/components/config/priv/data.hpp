#ifndef CONFIG_DATA_HPP
#define CONFIG_DATA_HPP

namespace key {
extern const char *PROVISIONING;
extern const char *WIFI_SSID;
extern const char *WIFI_PASSWD;
extern const char *PROV_NAME;
extern const char *MDNS_HOSTNAME;
extern const char *MDNS_INSTANCE;
extern const char *IS_BACKEND_LOCAL;
extern const char *BACKEND_MDNS_NAME;
extern const char *BACKEND_FQDN;
extern const char *LOG_LEVEL;
} // namespace key

namespace B {
extern const char *yes;
extern const char *no;
} // namespace B

namespace LOG {
extern const char *NONE;
extern const char *ERROR;
extern const char *WARN;
extern const char *INFO;
extern const char *DEBUG;
extern const char *VERBOSE;
} // namespace LOG

#endif
