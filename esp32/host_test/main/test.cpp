#include "cJSON.h"
#include "errors.hpp"
#include "esp_event.h"
#include "esp_log.h"
#include <algorithm>
#include <cstdlib>
#include <format>
#include <fstream>
#include <gtest/gtest.h>
#include <ranges>

using namespace std;
using std::ranges::for_each;
using std::views::enumerate;

namespace {
void *malloc_fn(size_t size) { return (void *)(new char[size]); }
void free_fn(void *ptr) { delete[] (char *)ptr; }
cJSON_Hooks hooks{.malloc_fn = &malloc_fn, .free_fn = &free_fn};

vector<string> get_args() {
    auto prog_name = std::string{"host_test"};
    string_view f = CONFIG_GTEST_FILTER;
    auto filter = std::format("--gtest_filter={}", f.empty() ? "*" : f);
    return std::vector{std::move(prog_name), std::move(filter)};
}
}; // namespace

extern "C" int app_main() {
    cJSON_InitHooks(&hooks);

    auto args = get_args();
    int argc = args.size();
    auto argv = new char *[argc];
    for (auto [i, arg] : enumerate(args))
        argv[i] = arg.data();

    ESP_LOGI("APP", "%s", argv[0]);
    ESP_LOGI("APP", "%s", argv[1]);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
