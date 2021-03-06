#pragma once
#include "lib/connect/tcp.h"
#include "lib/connect/task.h"
#include "lib/common/logging.h"
#include "lib/connect/connect_pool.h"
#include "got7/inner/context.h"
#include "got7/protobuf/feed.pb.h"
#include "got7/common/feed_utils.h"
using namespace qwb;

namespace got7 {
    class InnerRequestHandleTask : public TaskBase {
    public:
        InnerRequestHandleTask(InnerCtx *ctx, int innerFd, int outerFd);
        void constructEvent(EpollRun* manager) override;
        void readEvent(EpollRun* manager) override;
        void destructEvent(EpollRun* manager) override;

    private:
        InnerCtx *ctx;
        int innerFd, outerFd;
    };
}