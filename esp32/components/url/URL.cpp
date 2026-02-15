#include "URL.hpp"
#include <cstring>

URL::URL(std::string &&url) : url{std::move(url)} {
    http_parser_url_init(&parser);
    parse();
}

URL::URL(std::string_view url) : url{url} {
    http_parser_url_init(&parser);
    parse();
}

URL::URL(const char *url) : URL{std::string_view{url}} {}

std::string_view URL::get_hostname() const noexcept {
    if (!(parser.field_set & (1 << UF_HOST)))
        return {};
    auto host_len = parser.field_data[UF_HOST].len;
    auto host_off = parser.field_data[UF_HOST].off;

    return {url.data() + host_off, host_len};
}

const std::string &URL::get() const noexcept { return url; }

void URL::substitute_hostname(std::string_view new_hostname) noexcept {
    if (!(parser.field_set & (1 << UF_HOST)))
        return;
    auto offset = parser.field_data[UF_HOST].off;
    memcpy(url.data() + offset, new_hostname.data(), new_hostname.size());
}

void URL::parse() {
    int err = http_parser_parse_url(url.data(), url.size(), 0, &parser);
    UTIL_THROW(err, ValueError, "url parse");
}
