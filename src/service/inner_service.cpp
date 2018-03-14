#include <cstdio>
#include "lib/common/config.h"
#include "lib/connect/connect_pool.h"
using namespace qwb;

int main() {
    ConfigReaderPtr config = ConfigReaderFactory::createFromFile("/home/qwb/code/Clion/got7/config/inner_service.properties");
    int epoll_size = config->getAsInt("epoll_size", 3);
    int thread_size = config->getAsInt("thread_size", 2);
    int mainfd_size = config->getAsInt("mainfd_size", 2000);

    ConnectPollPtr pool(new ConnectPool(thread_size, epoll_size));
    pool->join();
    return 0;
}