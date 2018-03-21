#pragma once
#include "lib/common/base.h"
#include "got7/common/const.h"
using namespace qwb;

namespace got7 {
    class CtxBase {
    public:
        int pipeFd;
        int consumerId;

        u_char buff[Consts::BUFF_SIZE];
        uint16_t usedCount;
        int protoSize;

        uint64_t lastReadTime;

        CtxBase(int consumerId = 0):consumerId(consumerId){
            reset();
        }
        virtual ~CtxBase() = default;

        virtual void reset() {
            pipeFd = 0;
            usedCount = 0;
            protoSize = 0;
            lastReadTime = 0;
        }

        virtual void copyFrom(CtxBase *ctx) {
            // consumerId不复制
            int consumerId = this->consumerId;
            *this = *ctx;
            this->consumerId = consumerId;
        }
    };
}