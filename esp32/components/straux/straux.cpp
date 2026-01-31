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
} // namespace straux
