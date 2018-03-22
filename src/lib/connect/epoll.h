#pragma once
#include <sys/epoll.h>
#include "lib/common/base.h"
#include "lib/connect/task.h"
#include "lib/common/logging.h"

namespace qwb {
    class ConnectPool;

    class EpollRun {
    public:
        virtual ~EpollRun();
        void init(ConnectPool *connectPool, int epollSize, int consumerId);
        bool add(TaskBase *task, TaskEvents taskEvents);
        bool remove(TaskBase *task);
        bool loopOnce();

        void setLogName(const char *name) {
            if(log != nullptr) {
                log->setName(name);
            }
        }
        void setLogLevel(LogLevel level) {
            if(log != nullptr) {
                log->setLevel(level);
            }
        }
        void setWaitMs(int waitMs) {
            this->waitMs = waitMs;
        }
        ConnectPool* getConnectPool() {
            return connectPool;
        }

    protected:
        int waitMs = 10000;

        int consumerId;
        int epollFd, epollSize;
        ConnectPool *connectPool;

        std::vector<epoll_event> activeEvents;
        LoggerPtr log;
    };
}
