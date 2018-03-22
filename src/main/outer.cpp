#include "got7/outer/service.h"
using namespace got7;

int main() {
    OuterService service(1, 128);
    if(!service.prepare(4001)) {
        std::printf("run failed.");
        exit(1);
    }

    service.addRequestCenter("10.64.70.148", 80, 4002);
    service.addRequestCenter("119.29.233.22", 4928, 4003);
    service.join();
    return 0;
}