#include "BaseTest.hpp"
#include "MockEventGroup.hpp"
#include "MockOTA.hpp"
#include "MockWiFiManager.hpp"
#include "runtime.hpp"
#include <gtest/gtest.h>

using namespace std;

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
        auto egroup = mkuptr<MockEventGroup>();
        this->egroup = egroup.get();
        runtime = Runtime::create(std::move(cfg), move(egroup));
    }
    ~RuntimeTest() {}

    u_ptr<Runtime> runtime{};
    MockEventGroup *egroup{};
};

TEST_F(RuntimeTest, init) {}
