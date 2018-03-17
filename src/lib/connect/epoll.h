#pragma once
#include <sys/epoll.h>
#include "lib/common/base.h"
#include "lib/connect/task.h"
#include "lib/common/logging.h"

namespace qwb {
    class EpollManager {
    public:
        void init(int epoll_size, int consumer_id);
        void add(TaskPtr task, TaskEvents taskEvents);
        bool remove(const int fd);

    protected:
        const int waitMs = 5000;

        int consumer_id;
        int epoll_fd, epoll_size;
        std::map<int, TaskPtr> fd_map;

        std::vector<epoll_event> activeEvents;
        LoggerPtr log;
    };

    class EpollRun : public EpollManager {
    public:
        bool loop_once();
    };
}
