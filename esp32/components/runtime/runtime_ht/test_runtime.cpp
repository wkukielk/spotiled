#include "BaseTest.hpp"
#include "MockOTA.hpp"
#include "MockWiFiManager.hpp"
#include "runtime.hpp"
#include <gtest/gtest.h>

class RuntimeTest : public BaseTest {
  protected:
    RuntimeTest() {
        auto cfg = RuntimeConfig{
            .backend_cfg = {.base_url = URL{"https://backend.local:8000"},
                            .cert_pem = "cert",
                            .cert_len = 4,
                            .skip_cn_check = false},
            .wifi_mgr = mkuptr<MockWiFiManager>(),
            .ota = mkuptr<MockOTA>(),
        };
        runtime = Runtime::create(std::move(cfg));
    }
    ~RuntimeTest() {}

    u_ptr<Runtime> runtime{};
};

TEST_F(RuntimeTest, init) {}
