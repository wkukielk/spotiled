#ifndef URL_PARSER_HPP
#define URL_PARSER_HPP

#include "errors.hpp"
#include "http_parser.h"
#include "util.hpp"
#include <string>
#include <string_view>

class URL {
  public:
    explicit URL(std::string &&url);
    explicit URL(std::string_view url);
    explicit URL(const char *url);
    std::string_view get_hostname() const noexcept;
    const std::string &get() const noexcept;
    void substitute_hostname(std::string_view new_hostname) noexcept;

  private:
    void parse();
    http_parser_url parser{};
    std::string url{};
};

#endif
