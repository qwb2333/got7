#include "feed_utils.h"
using namespace got7;

idl::FeedAction FeedUtils::createPipe(int consumerId) {
    idl::FeedAction action;
    action.set_fd(consumerId);
    action.set_option(idl::FeedOption::PIPE);
    return action;
}

idl::FeedAction FeedUtils::createConnect(int fd, const char *ip, uint16_t port) {
    idl::FeedAction action;
    idl::FeedRemoteInfo *info = new idl::FeedRemoteInfo();
    info->set_ip(ip);
    info->set_port(port);

    action.set_fd(fd);
    action.set_option(idl::FeedOption::CONNECT);
    action.set_allocated_remoteinfo(info);
    return action;
}

idl::FeedAction FeedUtils::createDisconnect(int fd) {
    idl::FeedAction action;
    action.set_fd(fd);
    action.set_option(idl::FeedOption::DISCONNECT);
    return action;
}

idl::FeedAction FeedUtils::createAck(int consumerId) {
    idl::FeedAction action;
    action.set_fd(consumerId);
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

int FeedUtils::sendAction(idl::FeedAction &action, int pipeFd) {
    if(!pipeFd) {
        // ctx被清空了,pipeFd变成了0
        return false;
    }

    u_char buff[Consts::BUFF_SIZE];
    int byteSize = action.ByteSize();

    // 先发一个2字节的整数,表示protobuf的长度
    u_char *ptr = buff, *limit = buff + Consts::BUFF_SIZE;
    Utils::uint16ToUChars((uint16_t)byteSize, ptr); ptr += Consts::PROTO_LEN_SIZE;
    action.SerializeToArray(ptr, (int)(limit - ptr)); ptr += byteSize;
    int ret = (int)Tcp::write(pipeFd, buff, (unsigned)(ptr - buff));
    return ret;
}

int FeedUtils::readMessage(std::vector<idl::FeedAction> &refActionVec, CtxBase *ctx) {
    refActionVec.clear();

    if(!ctx->protoSize) {
        if(ctx->usedCount >= Consts::PROTO_LEN_SIZE) {
            // buff中是有protoSize大小的,弄一下
            ctx->protoSize = Utils::uCharsToUint16(ctx->buff);
        } else {
            int len = Tcp::read(ctx->pipeFd, ctx->buff + ctx->usedCount,
                                  (unsigned)(Consts::BUFF_SIZE - ctx->usedCount));
            if(len <= 0) return len;
            ctx->usedCount += len;

            if(ctx->usedCount < Consts::PROTO_LEN_SIZE) {
                // 这个情况非常奇葩, 应该不会出现才对
                LOG->error("can't get protoSize, pipeFd = %d.", ctx->pipeFd);
                return 0; // 认为需要断开
            }
            ctx->protoSize = Utils::uCharsToUint16(ctx->buff);
        }
        if(ctx->protoSize > Consts::PAGE_SIZE + 128) {
            // 这个情况理论是不存在的, protoSize理论只会稍微大于pageSize,因为protobuf中除了data还带有一些其他数据
            LOG->error("usedCount > pageSize. pipeFd = %d.", ctx->pipeFd);
            return 0; // 认为需要断开
        }
    }

    // proto内容还不足够,循环读
    int whileCount = 0;
    while(ctx->usedCount - Consts::PROTO_LEN_SIZE < ctx->protoSize) {
        if(whileCount >= 3) {
            // 理论不会循环这么多次啊
            LOG->error("whileCount > 3. pipeFd = %d, protoSize = %d, whileCount = %d",
                       ctx->pipeFd, ctx->protoSize, whileCount);
        }
        int len = Tcp::read(ctx->pipeFd, ctx->buff + ctx->usedCount,
                              (unsigned)(Consts::BUFF_SIZE - ctx->usedCount));
        if(len <= 0) return len;

        ctx->usedCount += len;
        whileCount++;
    }

    int resLen = ctx->usedCount, beginPos = 0;
    while(ctx->protoSize && beginPos + Consts::PROTO_LEN_SIZE + ctx->protoSize <= ctx->usedCount) {
        idl::FeedAction action;
        serializeFeedAction(action, ctx->buff + beginPos + Consts::PROTO_LEN_SIZE, ctx->protoSize);

        resLen -= Consts::PROTO_LEN_SIZE + ctx->protoSize;
        beginPos += ctx->protoSize + Consts::PROTO_LEN_SIZE;
        refActionVec.emplace_back(action);

        if(resLen < Consts::PROTO_LEN_SIZE) {
            ctx->protoSize = 0; // 长度已经不足够了
        } else {
            ctx->protoSize = Utils::uCharsToUint16(ctx->buff + beginPos);
        }
    }

    for(int i = 0; i < resLen; i++) {
        ctx->buff[i] = ctx->buff[i + beginPos];
    }
    ctx->usedCount = (uint16_t)resLen;

    // 更新lastReadTime时间
    ctx->lastReadTime = Utils::getTimeNow();

    return 1; // 表示正常返回
}
