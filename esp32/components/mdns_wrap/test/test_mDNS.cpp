#include "mDNS.hpp"
#include "unity.h"
#include "unity_fixture_extras.h"

namespace {
int magic;

void setup(void) { magic = 1; }

void teardown(void) { magic = 0; }

}; // namespace

TEST_CASE("write read", "[nvs]") { TEST_ASSERT_EQUAL(magic, 2); }
