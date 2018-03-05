#include "utils.h"
using namespace qwb;

std::string Utils::format(const char *fmt, ...) {
    char buffer[4096];
    char *p = buffer, *limit = buffer + sizeof(buffer);

    va_list args;
    va_start(args, fmt);
    p += vsnprintf(p, limit - p, fmt, args);
    va_end(args);

    p = std::min(p, limit - 1);
    *++p = 0;

    return std::string(buffer);
}
