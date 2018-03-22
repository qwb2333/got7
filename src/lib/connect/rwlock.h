#pragma once

#include <pthread.h>

namespace qwb {
    class AutoLock {
        pthread_mutex_t *_lock;
    public:
        AutoLock(pthread_mutex_t *lock) {
            _lock = lock;
            pthread_mutex_lock(_lock);
        }

        ~AutoLock() {
            pthread_mutex_unlock(_lock);
        }
    };

    class RWLock {
        pthread_rwlock_t _lock;
    public:
        RWLock() {
            pthread_rwlock_init(&_lock, NULL);
        }

        int read() {
            return pthread_rwlock_rdlock(&_lock);
        }

        int write() {
            return pthread_rwlock_wrlock(&_lock);
        }

        int unlock() {
            return pthread_rwlock_unlock(&_lock);
        }
    };


    class AutoRead {
        RWLock *_lock;
    public:
        AutoRead(RWLock *p) {
            _lock = p;
            _lock->read();
        }

        ~AutoRead() {
            _lock->unlock();
        }
    };

    class AutoWrite {
        RWLock *_lock;
    public:
        AutoWrite(RWLock *p) {
            _lock = p;
            _lock->write();
        }

        ~AutoWrite() {
            _lock->unlock();
        }
    };
}
