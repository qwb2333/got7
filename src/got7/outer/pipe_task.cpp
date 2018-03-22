#include "pipe_task.h"
using namespace got7;

OuterPipeCenterManager::OuterPipeCenterManager(OuterCtx *outerCtxArr,
                                               TcpPtr tcp, std::atomic<uint32_t> *acceptCount, int consumerId) {
    this->tcp = tcp;
    this->outerCtxArr = outerCtxArr;
    this->consumerId = consumerId;
    this->acceptCount = acceptCount;
}

void OuterPipeCenterManager::loopOnce() {
    RemoteInfo remoteInfo;
    if(!tcp->accept(remoteInfo)) {
        // 断开连接了
        log->info("OuterPipeCenterManager, CLOSED.");
        ::close(tcp->get_fd());
        return;
    }

    log->info("new accept %s:%d, fd = %d", remoteInfo.ip.c_str(), (int)remoteInfo.port, remoteInfo.fd);

    OuterCtx ctx(consumerId);
    idl::FeedAction action;

    ctx.pipeFd = remoteInfo.fd;
    Tcp::setSocketTimeout(ctx.pipeFd, 10); //设置超时
    if(FeedUtils::readMessage(action, (CtxBase*)&ctx) <= 0) {
        log->info("read PIPE failed.");
        ::close(ctx.pipeFd);
        return;
    }

    auto option = action.option();
    if(option == idl::FeedOption::PIPE) {
        int consumerId = action.fd();

        int pipeFd = remoteInfo.fd;
        action = FeedUtils::createAck();
        if(FeedUtils::sendAction(action, pipeFd) <= 0) {
            log->info("send ACK failed.");
            return;
        }

        OuterCtx &threadCtx = outerCtxArr[consumerId];
        std::unique_lock<std::mutex> uniqueLock(threadCtx.mutex);
        log->info("send ACK success.ready to update ctx.");

        // 取消超时
        Tcp::setSocketTimeout(ctx.pipeFd);

        threadCtx.copyFrom(&ctx);
        uniqueLock.unlock();
        threadCtx.condition.notify_one();

        return;
    }

    log->info("when accept, first message should PIPE.");
    ::close(ctx.pipeFd);
}


void OuterPipeHandleTask::readEvent(ConnectPool *manager) {
    idl::FeedAction action;
    int pipeFd = ctx->pipeFd;
    int len = FeedUtils::readMessage(action, ctx);

    if(len <= 0) {
        if(len == -1) {
            log->error("read fd = %d, errno = %d, %s", pipeFd, errno, strerror(errno));
        }
        return;
    }

    idl::FeedOption option = action.option();
    int outerFd = action.fd();

    if(option == idl::FeedOption::DISCONNECT) {
        log->info("recv DISCONNECT. outerFd = %d", outerFd);
        if(!ctx->fdExists.count(outerFd)) {
            log->info("had DISCONNECT. outerFd = %d", outerFd);
        } else {
            ::close(outerFd);
            ctx->fdExists.erase(outerFd);
        }
    } else if(option == idl::FeedOption::MESSAGE) {
        log->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
        // 收到Message的消息, 直接写到proty_port里去
        if(!ctx->fdExists.count(outerFd)) {
            log->info("had DISCONNECT. outerFd = ", outerFd);
        } else {
            ::write(outerFd, action.data().c_str(), action.data().length());
        }
    } else if(option == idl::FeedOption::ACK) {
        log->info("recv ACK.");

        action = FeedUtils::createAck();
        FeedUtils::sendAction(action, pipeFd);
        log->info("send ACK.");
    } else if(option == idl::FeedOption::PIPE) {
        // 理论这里应该是不会收到PIPE的
        log->warn("It shoud not get PIPE.");
    }
}

void OuterPipeHandleTask::destructEvent(ConnectPool *manager) {
    // TODO 安全退出
    // TODO 更安全的管理fd, 防止无用的fd一直打开着
    log->info("pipe DISCONNECT. fd = %d", fd);
    ::close(fd);
}

void OuterPipeHandleTask::forceDestructEvent(ConnectPool *manager) {
    log->info("force, pipe DISCONNECT. fd = %d", fd);
    ::close(fd);
}