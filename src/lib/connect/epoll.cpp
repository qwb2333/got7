#include "epoll.h"
using namespace qwb;

void EpollManager::init(int epoll_size) {
    this->epoll_size = epoll_size;
    activeEvents.resize(epoll_size);
    this->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if(this->epoll_fd < 0) {
        log->error("epoll create error %d %s", errno, strerror(errno));
    }
}

void EpollManager::add(const TaskBase &task) {
    struct epoll_event event;
    fd_map[task.fd] = task;
    event.data.ptr = &fd_map[task.fd];
    event.events = (uint32_t)task.fd;
    int r = epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, task.fd, &event);
    if(r < 0) {
        log->error("epoll add error %d %s", errno, strerror(errno));
    }
}

bool EpollManager::remove(const TaskBase &task) {
    struct epoll_event event;
    auto iter = fd_map.find(task.fd);
    if(iter == fd_map.end()) {
        return false;
    }

    event.data.ptr = &iter->second;
    int r = epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, task.fd, &event);
    fd_map.erase(task.fd);
    if(r < 0) {
        log->error("epoll add error %d %s", errno, strerror(errno));
    }
    return true;
}

bool EpollRun::loop_once() {
    int num = epoll_wait(this->epoll_fd, &activeEvents[0], this->epoll_size, -1);
    if(num < 0) {
        log->error("epoll wait error %d %s", errno, strerror(errno));
    }

    for(int i = 0; i < num; i++) {
        if(activeEvents[i].events & EPOLLIN) {
            TaskBase *task = static_cast<TaskBase*>(activeEvents[i].data.ptr);
            task->dealReadEvent();
        } else if(activeEvents[i].events & EPOLLOUT) {
            TaskBase *task = static_cast<TaskBase*>(activeEvents[i].data.ptr);
            task->dealWriteEvent();
        } else {
            log->warn("unexpeted poll events fd=%d, event=%d", activeEvents[i]);
        }
    }
    return true;
}