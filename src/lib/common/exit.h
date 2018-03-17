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
    void saveExit(int sig, const std::function<void()> &func) {
        handlers[sig] = func;
        ::signal(SIGINT, signal_handler);
    }
}