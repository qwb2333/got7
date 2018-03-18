#include "connect_pool.h"
using namespace qwb;

void ConnectPool::add(TaskPtr task, TaskEvents taskEvents) {
    int id = Utils::hash(task->fd, thread_size);
    epoll_run_pool[id].add(task, taskEvents);
}

void ConnectPool::remove(const int fd) {
    int id = Utils::hash(fd, thread_size);
    epoll_run_pool[id].remove(fd);
}

ConnectPool::ConnectPool(int thread_size, int epoll_size) {
    this->thread_size = thread_size;
    this->epoll_size = epoll_size;

    epoll_run_pool.resize((unsigned)thread_size);
    for(int i = 0; i < thread_size; i++) {
        epoll_run_pool[i].init(this, epoll_size, i);
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

void ConnectPool::real_run(EpollRun &epollRun, int id) {
    while(epollRun.loop_once());
    log->info("thread %d closed.", id);
}