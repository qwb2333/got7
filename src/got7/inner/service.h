#pragma once
#include "lib/connect/tcp.h"
#include "lib/common/utils.h"
#include "lib/connect/connect_pool.h"
#include "got7/inner/pipe_task.h"
#include "got7/common/const.h"
#include "got7/inner/context.h"
#include "got7/common/feed_utils.h"
using namespace qwb;

namespace got7 {
    class InnerService : public ConnectPool {
    public:
        InnerService(int threadSize, int epollSize)
                :ConnectPool(threadSize, epollSize) {
            for(int i = 0; i < threadSize; i++) {
                innerCtxVec.emplace_back(InnerCtx(i));
            }
        }

        void setOuterServiceInfo(std::string outerIp, uint16_t outerPipePort);

        // InnerService -> InnerPipeService & InnerRequestService
        void realRun(EpollRun &epollRun, int consumerId) override;

    private:
        std::string outerIp;
        uint16_t outerPipePort;
        std::vector<InnerCtx> innerCtxVec;

        bool requireNewPipe(TcpPtr tcp, InnerCtx *ctx, int consumerId);
    };
}