#pragma once
#include <thread>
#include "lib/connect/epoll.h"

namespace qwb {
    class ConnectPool {
    public:
        ConnectPool(int thread_size, int epoll_size);

        void add(TaskPtr task, TaskEvents taskEvents);
        void remove(const int fd);
        void join();

    private:
        int thread_size, epoll_size;
        std::vector<EpollRun> epoll_run_pool;

        void real_run(EpollRun &epollRun, int id);

        // 有一定的概率会出现直接取模哈希没有打散，所以这个函数是非常有必要的
        int hash(int x);
    };

    typedef std::unique_ptr<ConnectPool> ConnectPollPtr;
}