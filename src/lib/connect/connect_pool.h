#pragma once
#include <thread>
#include "lib/connect/epoll.h"
#include "lib/common/utils.h"

namespace qwb {
    class ConnectPool {
    public:
        ConnectPool(int threadSize, int epollSize);
        virtual ~ConnectPool() = default;

        void add(TaskBase *task, TaskEvents taskEvents);
        void remove(int fd);
        void join();

        EpollRun &getEpollRun(int consumerId) {
            return epollRunPool[consumerId];
        }
        int getThreadSize() const {
            return threadSize;
        }

    protected:
        int threadSize;
        std::vector<EpollRun> epollRunPool;

        virtual void realRun(EpollRun &epollRun, int id);
    };

    typedef std::unique_ptr<ConnectPool> ConnectPollPtr;
}