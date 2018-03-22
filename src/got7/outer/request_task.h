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
        OuterRequestCenterTask(OuterCtx *ctx, TcpPtr tcp, const char *innerProxyIp, uint16_t innerProxyPort) {
            ::srand((unsigned)time(NULL));
            this->ctx = ctx;
            this->tcp = tcp;
            this->fd = tcp->get_fd();
            this->innerProxyIp = innerProxyIp;
            this->innerProxyPort = innerProxyPort;
        }
        void readEvent(ConnectPool* manager) override;
        void destructEvent(ConnectPool* manager) override;
        void forceDestructEvent(ConnectPool *manager) override;

    private:
        TcpPtr tcp;
        OuterCtx *ctx;
        const char *innerProxyIp;
        uint16_t innerProxyPort;

        int randInt(int size) {
            return rand() % size;
        }
    };

    class OuterRequestHandleTask : public TaskBase {
    public:
        OuterRequestHandleTask(OuterCtx *ctx, int outerFd) {
            this->ctx = ctx;
            this->fd = outerFd;
        }
        void readEvent(ConnectPool* manager) override;
        void destructEvent(ConnectPool* manager) override;
        void forceDestructEvent(ConnectPool *manager) override;

    private:
        OuterCtx *ctx;
    };
}