#include "connect_pool.h"
using namespace qwb;

void ConnectPool::add(TaskPtr task, TaskEvents taskEvents) {
    int id = hash(task->fd);
    epoll_run_pool[id].add(task, taskEvents);
}

void ConnectPool::remove(const int fd) {
    int id = hash(fd);
    epoll_run_pool[id].remove(fd);
}

ConnectPool::ConnectPool(int thread_size, int epoll_size) {
    this->thread_size = thread_size;
    this->epoll_size = epoll_size;

    epoll_run_pool.resize((unsigned)thread_size);
    for(int i = 0; i < thread_size; i++) {
        epoll_run_pool[i].init(epoll_size, i);
    }
}

void ConnectPool::join() {
    std::thread thread_array[thread_size];
    for(int i = 0; i < thread_size; i++) {
        thread_array[i] = std::thread([this, i] {
            this->real_run(epoll_run_pool[i], i);
        });
    }

    for(int i = 0; i < thread_size; i++) {
        thread_array[i].join();
    }
}

int ConnectPool::hash(int x) {
    unsigned hash = 5381;
    while (x) {
        hash += (hash << 5) + x % 10;
        x /= 10;
    }
    return (hash & 0x7FFFFFFF) % thread_size;
}

void ConnectPool::real_run(EpollRun &epollRun, int id) {
    while(epollRun.loop_once());
    log->info("thread %d closed.", id);
}