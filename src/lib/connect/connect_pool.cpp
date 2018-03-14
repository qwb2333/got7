#include "connect_pool.h"
using namespace qwb;

void ConnectPool::add(const TaskBase &task) {
    int id = task.fd % thread_size;
    epoll_run_pool[id].add(task);
}

void ConnectPool::remove(const TaskBase &task) {
    int id = task.fd % thread_size;
    epoll_run_pool[id].remove(task);
}

ConnectPool::ConnectPool(int thread_size, int epoll_size) {
    this->thread_size = thread_size;
    this->epoll_size = epoll_size;

    epoll_run_pool.resize(thread_size);
    for(int i = 0; i < thread_size; i++) {
        epoll_run_pool[i].init(epoll_size);
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
    //log->info("thread %d closed.", id);
}