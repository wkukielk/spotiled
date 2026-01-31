#include "cJSON.h"
#include "errors.hpp"
#include "esp_event.h"
#include "esp_log.h"
#include <gtest/gtest.h>

namespace {
void *malloc_fn(size_t size) { return (void *)(new char[size]); }
void free_fn(void *ptr) { delete[] (char *)ptr; }
cJSON_Hooks hooks{.malloc_fn = &malloc_fn, .free_fn = &free_fn};
}; // namespace

extern "C" int app_main() {
    int argc = 1;
    char *argv[1] = {(char *)"host_test"};
    cJSON_InitHooks(&hooks);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
