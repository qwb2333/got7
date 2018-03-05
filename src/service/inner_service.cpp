#include <cstdio>
#include "lib/common/utils.h"
using namespace qwb;

int main() {
    std::printf(Utils::format("[%d], [%d]", 1, 2).c_str());
    return 0;
}