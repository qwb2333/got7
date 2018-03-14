#pragma once
#include <sys/epoll.h>
#include "lib/common/base.h"
#include "lib/connect/task.h"
#include "lib/common/logging.h"

namespace qwb {
    class EpollManager {
    public:
        void init(int epoll_size);
        void add(const TaskBase &task);
        bool remove(const TaskBase &task);

    protected:
        const int waitMs = 5000;

        int epoll_fd, epoll_size;
        std::map<int, TaskBase> fd_map;

        std::vector<epoll_event> activeEvents;
    };

    class EpollRun : public EpollManager {
    public:
        bool loop_once();
    };
}
