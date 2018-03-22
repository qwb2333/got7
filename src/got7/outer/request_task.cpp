#include "request_task.h"
using namespace got7;

void OuterRequestCenterTask::readEvent(EpollRun *manager) {
    RemoteInfo remoteInfo;
    if(!tcp->accept(remoteInfo)) {
        manager->remove(this);
        return;
    }

    log->info("new accept, outerFd = %d, %s:%d", remoteInfo.fd, remoteInfo.ip.c_str(), (int)remoteInfo.port);
    int outerFd = remoteInfo.fd;


    ConnectPool *connectPool = manager->getConnectPool();
    const int usedConsumerId = randInt(connectPool->getThreadSize());

    OuterCtx *ctx = &outerCtxArr[usedConsumerId];
    log->info("send CONNECT. outerFd = %d", outerFd);

    auto action = FeedUtils::createConnect(outerFd, innerProxyIp, innerProxyPort);
    FeedUtils::sendAction(action, ctx->pipeFd);

    TaskBase *outerRequestHandleTask = new OuterRequestHandleTask(&outerCtxArr[usedConsumerId], outerFd);
    connectPool->getEpollRun(usedConsumerId).add(outerRequestHandleTask, TaskEvents::ReadEvent);
}

void OuterRequestCenterTask::destructEvent(EpollRun *manager) {
    // 本身这个fd是不应该被remove掉的,所以这种情况应该是不会出现的
    log->warn("OuterRequestCenterTask DISCONNECT.");
    ::close(fd);
}

void OuterRequestHandleTask::constructEvent(EpollRun *manager) {
    log->info("add map, outerFd = %d", fd);

    TaskBase *taskBase = this;
    {
        AutoWrite lock(&ctx->rwLock);
        ctx->fdMap.insert(std::make_pair(fd, taskBase));
    }
}

void OuterRequestHandleTask::readEvent(EpollRun *manager) {
    u_char buff[Consts::PAGE_SIZE];

    idl::FeedAction action;
    int len = (int)::read(fd, buff, Consts::PAGE_SIZE);
    if(len <= 0) {
        if(len == -1) {
            log->error("read fd = %d, errno = %d, %s", ctx->pipeFd, errno, strerror(errno));
        }

        log->info("recv proxy DISCONNECT. send DISCONNECT. outerFd = %d", fd);
        action = FeedUtils::createDisconnect(fd);
        FeedUtils::sendAction(action, ctx->pipeFd);

        manager->remove(this);
        return;
    }

    log->info("recv proxy MESSAGE, len = %d; send MESSAGE. outerFd = %d", len, fd);
    FeedUtils::createMessage(action, fd, buff, len);
    FeedUtils::sendAction(action, ctx->pipeFd);
}

void OuterRequestHandleTask::destructEvent(EpollRun *manager) {
    {
        AutoWrite lock(&ctx->rwLock);
        ctx->fdMap.erase(fd);
    }
    ::close(fd);
}