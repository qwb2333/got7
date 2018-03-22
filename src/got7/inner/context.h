#pragma once
#include "got7/common/context.h"
using namespace got7;

namespace got7 {
    class InnerCtx: public CtxBase {
    public:
        std::map<int, int> fdMap;
        InnerCtx(int consumerId = 0): CtxBase(consumerId){ }
        ~InnerCtx() { }

        void reset() override {
            fdMap.clear();
            CtxBase::reset();
        }
    };
}
