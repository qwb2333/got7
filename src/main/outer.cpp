#include "got7/outer/service.h"
using namespace got7;

int main() {
    OuterService service(10, 2000);
    if(!service.prepare(4001)) {
        std::printf("run failed.");
        exit(1);
    }
    service.join();
    return 0;
}