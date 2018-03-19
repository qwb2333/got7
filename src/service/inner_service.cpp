#include <cstdio>
#include "lib/common/utils.h"
using namespace qwb;

int main() {
    u_char s[100];
    Utils::uint16ToUChars(2333, s);
    std::printf("%d\n", (int)Utils::uCharsToUint16(s));
    return 0;
}