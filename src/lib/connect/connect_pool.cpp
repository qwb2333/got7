#include "connect_pool.h"
using namespace qwb;

void ConnectPool::add(TaskBase *task, TaskEvents taskEvents) {
    int id = Utils::hash(task->fd, threadSize);
    epollRunPool[id].add(task, taskEvents);
}

void ConnectPool::remove(TaskBase *task) {
    int id = Utils::hash(task->fd, threadSize);
    epollRunPool[id].remove(task);
}

ConnectPool::ConnectPool(int threadSize, int epollSize) {
    this->threadSize = threadSize;

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
    LOG->info("thread %d closed.", id);
}
