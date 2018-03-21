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
        std::atomic<uint32_t> *acceptCount;
        int consumerId;
        TcpPtr tcp;
    };

    class OuterPipeHandleTask : public TaskBase {
    public:
        OuterPipeHandleTask(OuterCtx *ctx, int fd) {
            this->fd = fd;
            this->ctx = ctx;
        }
        void readEvent(ConnectPool* manager) override;
        void destructEvent(ConnectPool *manager) override;
        void forceDestructEvent(ConnectPool *manager) override;

    private:
        OuterCtx *ctx;
    };
}