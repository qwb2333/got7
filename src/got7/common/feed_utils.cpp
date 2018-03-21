#include "feed_utils.h"
using namespace got7;

idl::FeedAction FeedUtils::createPipe(int consumerId) {
    idl::FeedAction action;
    action.set_fd(consumerId);
    action.set_option(idl::FeedOption::PIPE);
    return action;
}

idl::FeedAction FeedUtils::createConnect(int fd) {
    idl::FeedAction action;
    action.set_fd(fd);
    action.set_option(idl::FeedOption::CONNECT);
    return action;
}

idl::FeedAction FeedUtils::createDisconnect(int fd) {
    idl::FeedAction action;
    action.set_fd(fd);
    action.set_option(idl::FeedOption::DISCONNECT);
    return action;
}

idl::FeedAction FeedUtils::createAck() {
    idl::FeedAction action;
    action.set_option(idl::FeedOption::ACK);
    return action;
}

void FeedUtils::createMessage(idl::FeedAction &action, int fd, const u_char *buff, int size) {
    action.set_fd(fd);
    action.set_data(buff, (unsigned)size);
    action.set_option(idl::FeedOption::MESSAGE);
}

void FeedUtils::serializeFeedAction(idl::FeedAction &action, const u_char *buff, int size) {
    action.ParseFromArray(buff, size);
}

bool FeedUtils::sendAction(idl::FeedAction &action, int pipeFd) {
    u_char buff[Consts::BUFF_SIZE];
    int byteSize = action.ByteSize();

    // 先发一个2字节的整数,表示protobuf的长度
    u_char *ptr = buff, *limit = buff + Consts::BUFF_SIZE;
    Utils::uint16ToUChars((uint16_t)byteSize, ptr); ptr += Consts::PROTO_LEN_SIZE;
    action.SerializeToArray(ptr, (int)(limit - ptr)); ptr += byteSize;
    int ret = (int)::write(pipeFd, buff, ptr - buff);
    return ret > 0;
}

int FeedUtils::readMessage(idl::FeedAction &refAction, CtxBase *ctx) {
    if(!ctx->protoSize) {
        if(ctx->usedCount >= Consts::PROTO_LEN_SIZE) {
            // buff中是有protoSize大小的,弄一下
            ctx->protoSize = Utils::uCharsToUint16(ctx->buff);
        } else {
            int len = (int)::read(ctx->pipeFd, ctx->buff + ctx->usedCount,
                                  (unsigned)(Consts::BUFF_SIZE - ctx->usedCount));
            if(len <= 0) return len;
            ctx->usedCount += len;

            if(ctx->usedCount < Consts::PROTO_LEN_SIZE) {
                // 这个情况非常奇葩, 应该不会出现才对
                log->error("can't get protoSize, pipeFd = %d.", ctx->pipeFd);
                return 0; // 认为需要断开
            }
            ctx->protoSize = Utils::uCharsToUint16(ctx->buff);
        }
        if(ctx->protoSize > Consts::PAGE_SIZE + 128) {
            // 这个情况理论是不存在的, protoSize理论只会稍微大于pageSize,因为protobuf中除了data还带有一些其他数据
            log->error("usedCount > pageSize. pipeFd = %d.", ctx->pipeFd);
            return 0; // 认为需要断开
        }

        // 更新lastReadTime时间
        ctx->lastReadTime = Utils::getTimeNow();
    }

    // proto内容还不足够,循环读
    int whileCount = 0;
    while(ctx->usedCount - Consts::PROTO_LEN_SIZE < ctx->protoSize) {
        if(whileCount >= 1) {
            // 这个情况理论是不存在的
            log->error("whileCount > 1. pipeFd = %d, protoSize = %d", ctx->pipeFd, ctx->protoSize);
            return 0;
        }
        int len = (int)::read(ctx->pipeFd, ctx->buff + ctx->usedCount,
                              (unsigned)(Consts::BUFF_SIZE - ctx->usedCount));
        if(len <= 0) return len;

        ctx->usedCount += len;
        whileCount++;
    }

    serializeFeedAction(refAction, ctx->buff + Consts::PROTO_LEN_SIZE, ctx->protoSize);
    int resLen = ctx->usedCount - Consts::PROTO_LEN_SIZE - ctx->protoSize;
    int beginPos = ctx->protoSize + Consts::PROTO_LEN_SIZE;

    for(int i = 0; i < resLen; i++) {
        ctx->buff[i] = ctx->buff[i + beginPos];
    }
    ctx->protoSize = 0;
    ctx->usedCount = (uint16_t)resLen;
    return 1; // 表示正常返回
}