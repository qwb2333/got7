#include "got7/outer/service.h"
using namespace got7;

int main() {
    OuterService service(5, 128);
    if(!service.prepare(4001)) {
        std::printf("run failed.");
        exit(1);
    }

    bool success = true;
    success &= service.addRequestCenter("10.64.70.148", 80, 4002);
    success &= service.addRequestCenter("119.29.233.22", 4928, 4003);
    success &= service.addRequestCenter("97.64.35.146", 443, 4004);
    if(!success) {
        std::printf("run failed.");
        exit(1);
    }

    service.join();
    return 0;
}