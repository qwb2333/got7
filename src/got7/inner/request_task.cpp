#include "request_task.h"

InnerRequestHandleTask::InnerRequestHandleTask(InnerCtx *ctx, int innerFd, int outerFd) {
    this->fd = innerFd;

    this->ctx = ctx;
    this->innerFd = innerFd;
    this->outerFd = outerFd;
}

void InnerRequestHandleTask::readEvent(ConnectPool *manager) {
    u_char buff[Consts::PAGE_SIZE];

    idl::FeedAction action;
    int len = (int)::read(fd, buff, Consts::PAGE_SIZE);
    if(len <= 0) {
        if(len == -1) {
            // 这个情况没有遇到过..
            log->warn("len == -1. fd = %d, pipeFd = %d", fd, ctx->pipeFd);
        }
        manager->removeById(this, ctx->consumerId);
        return;
    }

    log->info("recv proxy MESSAGE, len = %d; send MESSAGE. outerFd = %d", len, outerFd);
    FeedUtils::createMessage(action, outerFd, buff, len);
    FeedUtils::sendAction(action, ctx->pipeFd);
}

void InnerRequestHandleTask::destructEvent(ConnectPool *manager) {
    idl::FeedAction action = FeedUtils::createDisconnect(outerFd);
    log->info("recv proxy DISCONNECT. send DISCONNECT. outerFd = %d", outerFd);

    FeedUtils::sendAction(action, ctx->pipeFd);
    ctx->fdMap.erase(outerFd);
    ::close(innerFd);
}

void InnerRequestHandleTask::forceDestructEvent(ConnectPool *manager) {
    // 暴力退出时,不发消息给对方
    log->info("force proxy DISCONNECT. outerFd = %d", outerFd);
    ctx->fdMap.erase(outerFd);
    ::close(innerFd);
}