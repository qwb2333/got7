#pragma once
#include <sys/epoll.h>

namespace qwb {
    class ConnectPool;

    class TaskBase {
    public:
        int fd;
        virtual ~TaskBase() = default;

        virtual void dealReadEvent(ConnectPool* manager) {

        }

        virtual void dealWriteEvent(ConnectPool* manager) {

        }
    };
    typedef std::shared_ptr<TaskBase> TaskPtr;

    enum TaskEvents {
        ReadEvent = EPOLLIN|EPOLLHUP|EPOLLERR,
        WriteEvent = EPOLLOUT|EPOLLHUP|EPOLLERR,
        All = EPOLLIN|EPOLLOUT|EPOLLHUP|EPOLLERR
    };
}