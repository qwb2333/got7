#pragma once
#include <thread>
#include "lib/connect/epoll.h"

namespace qwb {
    class ConnectPool {
    public:
        ConnectPool(int thread_size, int epoll_size);

        void add(const TaskBase &task);
        void remove(const TaskBase &task);
        void join();

    private:
        int thread_size, epoll_size;
        std::vector<EpollRun> epoll_run_pool;

        void real_run(EpollRun &epollRun, int id);
    };

    typedef std::unique_ptr<ConnectPool> ConnectPollPtr;
}