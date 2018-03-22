#pragma once
#include <sys/epoll.h>

namespace qwb {
    class ConnectPool;

    class TaskBase {
    public:
        int fd;
        virtual ~TaskBase() {
            fd = -1; //表示没有初始化
        }

        virtual void readEvent(ConnectPool* manager) {

        }

        virtual void writeEvent(ConnectPool* manager) {

        }

        virtual void constructEvent(ConnectPool* manager) {

        }

        virtual void destructEvent(ConnectPool* manager) {

        }

        virtual void forceDestructEvent(ConnectPool *manager) {

        }

        bool removeAble = true;
    };

    enum TaskEvents {
        ReadEvent = EPOLLIN|EPOLLHUP|EPOLLERR,
        WriteEvent = EPOLLOUT|EPOLLHUP|EPOLLERR,
        All = EPOLLIN|EPOLLOUT|EPOLLHUP|EPOLLERR
    };
}