#include "connect_pool.h"
using namespace qwb;

void ConnectPool::add(TaskBase *task, TaskEvents taskEvents) {
    int id = Utils::hash(task->fd, threadSize);
    epollRunPool[id].add(task, taskEvents);
}

void ConnectPool::addById(TaskBase *task, TaskEvents taskEvents, int consumerId) {
    epollRunPool[consumerId].add(task, taskEvents);
}

void ConnectPool::remove(TaskBase *task, bool force) {
    int id = Utils::hash(task->fd, threadSize);
    epollRunPool[id].remove(task, force);
}

void ConnectPool::removeById(TaskBase *task, int consumerId, bool force) {
    epollRunPool[consumerId].remove(task, force);
}

ConnectPool::ConnectPool(int threadSize, int epollSize) {
    this->threadSize = threadSize;
    this->epollSize = epollSize;

    epollRunPool.resize((unsigned)threadSize);
    for(int i = 0; i < threadSize; i++) {
        epollRunPool[i].init(this, epollSize, i);
    }
}

void ConnectPool::join() {
    std::thread threadArray[threadSize];
    for(int i = 0; i < threadSize; i++) {
        threadArray[i] = std::thread([this, i] {
            this->realRun(epollRunPool[i], i);
        });
    }

    for(int i = 0; i < threadSize; i++) {
        threadArray[i].join();
    }
}

void ConnectPool::realRun(EpollRun &epollRun, int id) {
    while(epollRun.loopOnce());
    log->info("thread %d closed.", id);
}