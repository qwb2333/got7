#include "got7/inner/service.h"
using namespace got7;

int main() {
    InnerService service(10, 2000);
    service.setOuterServiceInfo("127.0.0.1", 4001);
    service.join();
    return 0;
}