#pragma once
#include <thread>
#include "lib/connect/epoll.h"
#include "lib/common/utils.h"

namespace qwb {
    class ConnectPool {
    public:
        ConnectPool(int thread_size, int epoll_size);

        void add(TaskPtr task, TaskEvents taskEvents);
        void remove(const int fd);
        void join();

        int getThreadSize() const {
            return thread_size;
        }

    private:
        int thread_size, epoll_size;
        std::vector<EpollRun> epoll_run_pool;

        void real_run(EpollRun &epollRun, int id);
    };

    typedef std::unique_ptr<ConnectPool> ConnectPollPtr;
}