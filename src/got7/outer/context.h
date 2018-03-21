#pragma once
#include <mutex>
#include <condition_variable>
#include "got7/common/context.h"
using namespace got7;

namespace got7 {
    class OuterCtx: public CtxBase {
    public:
        std::mutex mutex;
        std::condition_variable condition;
        std::set<int> fdExists;

        OuterCtx(int consumerId = 0): CtxBase(consumerId){ }
        ~OuterCtx() { }

        void reset() override {
            fdExists.clear();
            CtxBase::reset();
        }

        void copyFrom(OuterCtx *ctx) {
            CtxBase::copyFrom(ctx);
            this->fdExists = ctx->fdExists;
        }
    };
}
