#include "epoll.h"
using namespace qwb;

void EpollRun::init(ConnectPool *connectPool, int epoll_size, int consumer_id) {
    this->epoll_size = epoll_size;
    this->consumer_id = consumer_id;
    this->connectPool = connectPool;

    std::string name = std::string("EpollManager-" + std::to_string(consumer_id));
    log = LoggerFactory::createToStderr(name.c_str());

    activeEvents.resize((unsigned)epoll_size);
    this->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if(this->epoll_fd < 0) {
        log->error("epoll create error %d %s", errno, strerror(errno));
    }
}

void EpollRun::add(TaskPtr task, TaskEvents taskEvents) {
    struct epoll_event event;
    fd_map[task->fd] = task;
    event.data.ptr = &fd_map[task->fd];
    event.events = taskEvents;

    fcntl(task->fd, F_SETFL, O_NONBLOCK);
    log->info("epoll add task: %d", task->fd);
    int r = epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, task->fd, &event);
    if(r < 0) {
        log->error("epoll add error %d %s", errno, strerror(errno));
    }
}

bool EpollRun::remove(const int fd) {
    struct epoll_event event;
    auto iter = fd_map.find(fd);
    if(iter == fd_map.end()) {
        return false;
    }

    log->info("epoll remove task: %d", fd);
    event.data.ptr = &iter->second;
    int r = epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, &event);
    fd_map.erase(fd);
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

    log->info("epoll get %d events", num);
    for(int i = 0; i < num; i++) {
        int events = activeEvents[i].events;
        TaskPtr task = *static_cast<TaskPtr*>(activeEvents[i].data.ptr);

        bool success = false;
        if(events & EPOLLIN) {
            success = true;
            log->info("fd: %d, EPOLLIN", task->fd);
            task->dealReadEvent(this->connectPool);
        } else if(events & EPOLLOUT) {
            success = true;
            log->info("fd: %d, EPOLLOUT", task->fd);
            task->dealWriteEvent(this->connectPool);
        }

        if(events & EPOLLHUP) {
            success = true;
            log->info("fd: %d, EPOLLHUP", task->fd);
            remove(task->fd);
        }

        if(!success) {
            // 这是一条未处理的数据
            log->warn("unexpeted poll events fd=%d, event=%d", task->fd, activeEvents[i].events);
        }
    }
    return true;
}