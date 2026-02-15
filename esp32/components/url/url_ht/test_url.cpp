#include "URL.hpp"
#include <gtest/gtest.h>

class URLTest : public testing::Test {
  protected:
    URLTest() {}
};

TEST_F(URLTest, getHostname) {
    auto url = URL{"https://localhost:8000/users?name=Hammond"};
    ASSERT_EQ(url.get_hostname(), "localhost");
}

TEST_F(URLTest, getHostnameMDNS) {
    auto url = URL{"https://myesp32.local"};
    auto hostname = url.get_hostname();
    ASSERT_EQ(hostname, "myesp32.local");
    ASSERT_TRUE(hostname.ends_with(".local"));
}

TEST_F(URLTest, getHostnameIP) {
    auto url = URL{"https://192.168.43.21"};
    ASSERT_EQ(url.get_hostname(), "192.168.43.21");
}

TEST_F(URLTest, substituteHostname) {
    auto url = URL{"https://myesp32.local/index.html"};
    url.substitute_hostname("192.168.43.21");
    ASSERT_EQ(url.get(), "https://192.168.43.21/index.html");
}
