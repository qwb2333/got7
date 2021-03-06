#pragma once
#include "lib/connect/tcp.h"
#include "lib/common/base.h"
#include "lib/common/utils.h"
#include "lib/common/logging.h"
#include "got7/common/const.h"
#include "got7/common/context.h"
#include "got7/protobuf/feed.pb.h"
using namespace qwb;
using namespace got7;

namespace got7 {
    class FeedUtils {
    public:
        static idl::FeedAction createPipe(int consumerId);
        static idl::FeedAction createConnect(int fd, const char *ip, uint16_t port);
        static idl::FeedAction createDisconnect(int fd);
        static idl::FeedAction createAck(int consumerId);
        static void createMessage(idl::FeedAction &action, int fd, const u_char *buff, int size);
        static void serializeFeedAction(idl::FeedAction &action, const u_char *buff, int size);
        static int sendAction(idl::FeedAction &action, int pipeFd);
        static int readMessage(std::vector<idl::FeedAction> &refActionVec, CtxBase *ctx);
    };
}