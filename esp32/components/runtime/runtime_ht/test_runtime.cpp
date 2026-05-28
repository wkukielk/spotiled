#include "BaseTest.hpp"
#include "MockEventGroup.hpp"
#include "MockOTA.hpp"
#include "MockWiFiManager.hpp"
#include "runtime.hpp"
#include <gtest/gtest.h>

using namespace std;

class RuntimeTest : public BaseTest {
    //   protected:
    //     RuntimeTest() : runtime{Runtime::create(std::move(cfg))}, {}
    //     ~RuntimeTest() {}

    //     static RuntimeConfig get_cfg() {
    //         return RuntimeConfig{
    //             .backend_cfg = {.base_url =
    //             URL{"https://backend.local:8000"},
    //                             .cert_pem = "cert",
    //                             .cert_len = 4,
    //                             .skip_cn_check = false},
    //             .wifi_mgr = mkuptr<MockWiFiManager>()};
    //     }

    //     u_ptr<Runtime> runtime{};
};

TEST_F(RuntimeTest, init) {}
