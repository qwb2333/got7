#pragma once
#include "lib/connect/tcp.h"
#include "lib/connect/task.h"
#include "lib/common/logging.h"
#include "lib/connect/connect_pool.h"
#include "got7/outer/context.h"
#include "got7/protobuf/feed.pb.h"
#include "got7/common/feed_utils.h"
using namespace qwb;

namespace got7 {
    class OuterRequestCenterTask : public TaskBase {
    public:
        OuterRequestCenterTask(OuterCtx *outerCtxArr, int consumerId,
                               TcpPtr tcp, const char *innerProxyIp, uint16_t innerProxyPort) {
            ::srand((unsigned)time(NULL));
            this->tcp = tcp;
            this->fd = tcp->get_fd();
            this->consumerId = consumerId;
            this->outerCtxArr = outerCtxArr;
            this->innerProxyIp = innerProxyIp;
            this->innerProxyPort = innerProxyPort;
        }
        void readEvent(EpollRun *manager) override;
        void destructEvent(EpollRun *manager) override;

    private:
        TcpPtr tcp;
        int consumerId;
        OuterCtx *outerCtxArr;
        const char *innerProxyIp;
        uint16_t innerProxyPort;

        int randInt(int size) {
            return rand() % size;
        }
    };

    class OuterRequestHandleTask : public TaskBase {
    public:
        bool hadLocked;

        OuterRequestHandleTask(OuterCtx *ctx, int outerFd) {
            this->ctx = ctx;
            this->fd = outerFd;
            this->hadLocked = false;
        }
        void constructEvent(EpollRun *manager) override;
        void readEvent(EpollRun *manager) override;
        void destructEvent(EpollRun *manager) override;

    private:
        OuterCtx *ctx;
    };
}