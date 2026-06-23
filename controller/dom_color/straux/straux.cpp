#include "straux.hpp"
#include <algorithm>
#include <cstring>
#include <stdarg.h>

namespace straux
{
    char *strapp(char *dest, const char *source)
    {
        while (char c = *source++)
        {
            *dest++ = c;
        }
        *dest = '\0';
        return dest;
    }

    char *strcopy(const char *source)
    {
        size_t len = strlen(source) + 1;
        char *copy = new char[len];
        memcpy(copy, source, len);
        return copy;
    }
} // namespace straux
