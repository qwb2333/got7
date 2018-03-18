#pragma once
#include "lib/common/base.h"

namespace qwb {
    class Utils {
    public:
        static std::string format(const char *fmt, ...);
        static bool isBlank(const char &x);
        static std::string trim(char *buffer, size_t begin, size_t end);
        static int hash(int x, int mod);
    };
}