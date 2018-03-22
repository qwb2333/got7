#pragma once
#include <atomic>
#include "lib/connect/tcp.h"
#include "lib/connect/task.h"
#include "lib/common/logging.h"
#include "got7/outer/context.h"
#include "got7/outer/request_task.h"
#include "lib/connect/connect_pool.h"
#include "got7/protobuf/feed.pb.h"
#include "got7/common/feed_utils.h"
using namespace qwb;

namespace got7 {
    class OuterPipeCenterManager {
    public:
        OuterPipeCenterManager(OuterCtx *outerCtxArr,
                                  TcpPtr tcp, std::atomic<uint32_t> *acceptCount, int consumerId);
        void loopOnce();

    private:
        OuterCtx *outerCtxArr;

        // acceptCount暂时在这个类里没用,之后如果写了client动态添加端口,才会有用
        std::atomic<uint32_t> *acceptCount;
        int consumerId;
        TcpPtr tcp;
    };

    class OuterPipeHandleTask : public TaskBase {
    public:
        OuterPipeHandleTask(OuterCtx *ctx) {
            this->ctx = ctx;
            this->fd = ctx->pipeFd;
        }
        void readEvent(EpollRun* manager) override;
        void destructEvent(EpollRun *manager) override;

    private:
        OuterCtx *ctx;
    };
}