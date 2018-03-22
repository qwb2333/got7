#pragma once
#include <mutex>
#include <condition_variable>
#include "got7/common/context.h"
#include "lib/connect/rwlock.h"
using namespace got7;

namespace got7 {
    class OuterCtx: public CtxBase {
    public:
        std::mutex mutex;
        std::condition_variable condition;

        RWLock rwLock; //用于跨线程修改fdMap
        std::map<int, TaskBase*> fdMap;

        OuterCtx(int consumerId = 0): CtxBase(consumerId){ }
        ~OuterCtx() { }

        void reset() override {
            fdMap.clear();
            CtxBase::reset();
        }

        void copyFrom(OuterCtx *ctx) {
            CtxBase::copyFrom(ctx);
        }
    };
}
