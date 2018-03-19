#pragma once
#include "lib/common/base.h"

namespace qwb {
    class Utils {
    public:
        static std::string format(const char *fmt, ...);
        static bool isBlank(const char &x);
        static std::string trim(char *buffer, size_t begin, size_t end);
        static int hash(int x, int mod);
        static void uint16ToUChars(uint16_t x, u_char *number);
        static uint16_t uCharsToUint16(u_char *number);
    };
}