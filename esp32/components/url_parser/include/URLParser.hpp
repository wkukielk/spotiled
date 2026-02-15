#ifndef URL_PARSER_HPP
#define URL_PARSER_HPP

#include "http_parser.h"
#include <memory>
#include <string_view>

class URL {
  public:
    URL(std::string &&url) noexcept;
    URL(std::string_view url) noexcept;

  private:
    http_parser_url parser{};
};

#endif
