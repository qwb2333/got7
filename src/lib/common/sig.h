#pragma once
#include <signal.h>
#include "lib/common/base.h"

namespace {
    std::map<int, std::function<void()>> handlers;
    void signal_handler(int sig) {
        handlers[sig]();
    }
}

namespace qwb {
    class Signal {
    public:
        static void saveExit(int sig, const std::function<void()> &func) {
            handlers[sig] = func;
            ::signal(SIGINT, signal_handler);
        }

        // 屏蔽SIGPIPE信号
        static void ignoreSigPipe() {
            sigset_t signal_mask;
            sigemptyset(&signal_mask);
            sigaddset(&signal_mask, SIGPIPE);
            int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
            if (rc != 0) {
                printf("block sigpipe error\n");
            }
        }
    };
}