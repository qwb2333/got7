#pragma once
#include <atomic>
#include "lib/connect/tcp.h"
#include "lib/common/utils.h"
#include "lib/connect/connect_pool.h"
#include "got7/outer/pipe_task.h"
#include "got7/common/const.h"
#include "got7/outer/context.h"
#include "got7/common/feed_utils.h"
using namespace qwb;

namespace got7 {
    class OuterService : public ConnectPool {
    public:
        OuterService(int threadSize, int epollSize)
                :ConnectPool(threadSize, epollSize) {
            outerCtxArr = new OuterCtx[threadSize];
            for(int i = 0; i < threadSize; i++) {
                outerCtxArr[i].consumerId = i;
            }
        }

        ~OuterService() {
            delete outerCtxArr;
        }

        bool prepare(uint16_t outerPipePort);
        void realRun(EpollRun &epollRun, int consumerId) override;

    private:
        std::atomic<uint32_t> acceptCount;
        OuterCtx *outerCtxArr;
        bool requireNewPipe(OuterCtx *ctx);
    };
}