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

bool Utils::isBlank(const char &x) {
    return (x == ' ' || x == '\n' || x == '\r' || x == '\t');
}

std::string Utils::trim(char *buffer, size_t begin, size_t end) {
    while(begin <= end) {
        if(isBlank(buffer[begin])) begin++;
        else break;
    }
    while(begin <= end) {
        if(isBlank(buffer[end])) end--;
        else break;
    }

    char x = buffer[end + 1];
    buffer[end + 1] = 0;
    std::string str = std::string(buffer + begin);
    buffer[end + 1] = x;
    return str;
}

int Utils::hash(int x, int mod) {
    unsigned hash = 5381;
    while (x) {
        hash += (hash << 5) + x % 10;
        x /= 10;
    }
    return (hash & 0x7FFFFFFF) % mod;
}

void Utils::uint16ToUChars(uint16_t x, u_char *number) {
    int pos = 16;
    for(int i = 0; i < 2; i++) {
        pos -= 8;
        number[i] = (u_char)(x >> pos & 255);
    }
}

uint16_t Utils::uCharsToUint16(u_char *number) {
    int pos = 16;
    uint16_t ret = 0;
    for(int i = 0; i < 2; i++) {
        pos -= 8;
        ret |= number[i] << pos;
    }
    return ret;
}

uint64_t Utils::getTimeNow() {
    struct timeval now_tv;
    gettimeofday(&now_tv, NULL);
    return (uint64_t)now_tv.tv_sec;
}