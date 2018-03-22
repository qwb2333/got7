#pragma once
#include <sys/epoll.h>

namespace qwb {
    class EpollRun;

    class TaskBase {
    public:
        int fd;
        virtual ~TaskBase() {
            fd = -1; //表示没有初始化
        }

        virtual void readEvent(EpollRun* manager) {

        }

        virtual void writeEvent(EpollRun* manager) {

        }

        virtual void constructEvent(EpollRun* manager) {

        }

        virtual void destructEvent(EpollRun* manager) {

        }
    };

    enum TaskEvents {
        ReadEvent = EPOLLIN|EPOLLHUP|EPOLLERR,
        WriteEvent = EPOLLOUT|EPOLLHUP|EPOLLERR,
        All = EPOLLIN|EPOLLOUT|EPOLLHUP|EPOLLERR
    };
}