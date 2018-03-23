#pragma once

#include <pthread.h>

namespace qwb {
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
