#include "request_task.h"

InnerRequestHandleTask::InnerRequestHandleTask(InnerCtx *ctx, int innerFd, int outerFd) {
    this->fd = innerFd;

    this->ctx = ctx;
    this->innerFd = innerFd;
    this->outerFd = outerFd;
}

void InnerRequestHandleTask::readEvent(EpollRun *manager) {
    u_char buff[Consts::PAGE_SIZE];

    idl::FeedAction action;
    int len = (int)::read(fd, buff, Consts::PAGE_SIZE);
    if(len <= 0) {
        if(len == -1) {
            LOG->error("read fd = %d, error = %d, %s", ctx->pipeFd, errno, strerror(errno));
        }

        LOG->info("recv proxy DISCONNECT. send DISCONNECT. outerFd = %d", outerFd);
        action = FeedUtils::createDisconnect(outerFd);
        FeedUtils::sendAction(action, ctx->pipeFd);

        manager->remove(this);
        return;
    }

    LOG->info("recv proxy MESSAGE, len = %d; send MESSAGE. outerFd = %d", len, outerFd);
    FeedUtils::createMessage(action, outerFd, buff, len);
    FeedUtils::sendAction(action, ctx->pipeFd);
}

void InnerRequestHandleTask::constructEvent(EpollRun *manager) {
    LOG->info("add map, innerFd = %d, outerFd = %d", innerFd, outerFd);

    TaskBase *taskBase = this;
    std::pair<int, TaskBase*> value(innerFd, taskBase);
    ctx->fdMap.insert(std::make_pair(outerFd, value));
}

void InnerRequestHandleTask::destructEvent(EpollRun *manager) {
    ctx->fdMap.erase(outerFd);
    ::close(fd);
}
