#include "epoll.h"
using namespace qwb;

void EpollRun::reset() {
    log->warn("EpollRun reset begin.");
    auto iter = fdMap.begin();
    while(iter != fdMap.end()) {
        // 有些fd,是不允许随便remove的
        if(iter->second->removeAble) {
            remove(iter->second, true);
        }
        iter = fdMap.begin();
    }
}

EpollRun::~EpollRun() {
    this->reset();
}

void EpollRun::init(ConnectPool *connectPool, int epollSize, int consumerId) {
    this->epollSize = epollSize;
    this->consumerId = consumerId;
    this->connectPool = connectPool;

    std::string name = std::string("EpollManager-" + std::to_string(consumerId));
    log = LoggerFactory::createToStderr(name.c_str());

    activeEvents.resize((unsigned)epollSize);
    this->epollFd = epoll_create1(EPOLL_CLOEXEC);
    if(this->epollFd < 0) {
        log->error("epoll create error %d %s", errno, strerror(errno));
    }
}

void EpollRun::add(TaskBase *task, TaskEvents taskEvents) {
    struct epoll_event event;
    fdMap[task->fd] = task;
    event.data.ptr = task;
    event.events = taskEvents;

    fcntl(task->fd, F_SETFL, O_NONBLOCK);
    log->info("epoll add task: %d", task->fd);
    int r = epoll_ctl(this->epollFd, EPOLL_CTL_ADD, task->fd, &event);

    task->constructEvent(this->connectPool);
    if(r < 0) {
        log->error("epoll add error %d %s", errno, strerror(errno));
    }
}

bool EpollRun::remove(TaskBase *task, bool force) {
    struct epoll_event event;
    int fd = task->fd;

    auto iter = fdMap.find(fd);
    if(iter == fdMap.end()) {
        log->warn("not found in fdMap, fd = %d", fd);
        return false;
    }

    log->info("epoll remove task: %d", fd);
    event.data.ptr = &iter->second;
    int r = epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, &event);
    fdMap.erase(fd);

    if(force) {
        task->forceDestructEvent(this->connectPool);
    }
    else {
        task->destructEvent(this->connectPool);
    }

    if(r < 0) {
        log->error("epoll remove error, errno = %d, %s", errno, strerror(errno));
    }
    delete task; //这个task已经不需要了
    return true;
}

bool EpollRun::loopOnce() {
    // 一次epoll操作最多60s
    int num = epoll_wait(this->epollFd, &activeEvents[0], this->epollSize, 60000);
    if(num < 0) {
        log->error("epoll wait error, errno = %d, %s", errno, strerror(errno));
    }

    log->info("epoll get %d events", num);
    for(int i = 0; i < num; i++) {
        int events = activeEvents[i].events;
        TaskBase *task = static_cast<TaskBase*>(activeEvents[i].data.ptr);

        bool success = false;
        if(events & EPOLLIN) {
            success = true;
            log->info("fd: %d, EPOLLIN", task->fd);
            task->readEvent(this->connectPool);
        } else if(events & EPOLLOUT) {
            success = true;
            log->info("fd: %d, EPOLLOUT", task->fd);
            task->writeEvent(this->connectPool);
        }

        if(events & EPOLLHUP) {
            success = true;
            log->info("fd: %d, EPOLLHUP", task->fd);
            remove(task);
        }

        if(!success) {
            // 这是一条未处理的数据
            log->warn("unexpeted poll events fd=%d, event=%d", task->fd, activeEvents[i].events);
        }
    }
    return true;
}