#include "straux.hpp"
#include "esp_err.h"
#include <algorithm>
#include <cstdarg>
#include <cstring>

namespace straux {
char *strapp(char *dest, const char *source) {
    while (char c = *source++) {
        *dest++ = c;
    }
    *dest = '\0';
    return dest;
}

char *strcopy(const char *source) {
    size_t len = strlen(source) + 1;
    char *copy = new char[len];
    memcpy(copy, source, len);
    return copy;
}

void *strtoarr(void *dest, std::string_view source, size_t n) {
    memcpy(dest, source.data(), std::min(source.size(), n - 1));
    ((uint8_t *)dest)[n - 1] = 0;
    return dest;
}

} // namespace straux
