#include <cstdio>
#include "lib/common/config.h"
#include "lib/common/logging.h"
using namespace qwb;

int main() {
    ConfigReaderPtr config = ConfigReaderFactory::createFromFile("/home/qwb/code/Clion/got7/config/inner_service.properties");
    std::string thread_num = config->getAsString("thread_num");
    int mainfd_num = config->getAsInt("mainfd_num");

    LoggerPtr logger = LoggerFactory::createToStdout();
    logger->setLevel(LogLevel::WARN);
    printf("thread_num: %s, mainfd_num: %d\n", thread_num.c_str(), mainfd_num);
    return 0;
}