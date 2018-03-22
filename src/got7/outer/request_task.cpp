#include "request_task.h"
using namespace got7;

void OuterRequestCenterTask::readEvent(ConnectPool *manager) {
    RemoteInfo remoteInfo;
    if(!tcp->accept(remoteInfo)) {
        manager->removeById(this, ctx->consumerId);
        return;
    }

    log->info("new accept, outerFd = %d, %s:%d", remoteInfo.fd, remoteInfo.ip.c_str(), (int)remoteInfo.port);
    int outerFd = remoteInfo.fd;

    ctx->fdExists.insert(outerFd);

    auto action = FeedUtils::createConnect(outerFd, innerProxyIp, innerProxyPort);
    FeedUtils::sendAction(action, ctx->pipeFd);

    const int usedConsumerId = randInt(manager->getThreadSize());
    TaskBase *outerRequestHandleTask = new OuterRequestHandleTask(ctx, remoteInfo.fd);
    manager->addById(outerRequestHandleTask, TaskEvents::ReadEvent, usedConsumerId);
}

void OuterRequestCenterTask::destructEvent(ConnectPool *manager) {
    log->warn("OuterRequestCenterTask DISCONNECT.");
    ::close(fd);
}

void OuterRequestCenterTask::forceDestructEvent(ConnectPool *manager) {
    log->warn("OuterRequestCenterTask force DISCONNECT.");
    ::close(fd);
}


void OuterRequestHandleTask::readEvent(ConnectPool *manager) {
    u_char buff[Consts::PAGE_SIZE];

    idl::FeedAction action;
    int len = (int)::read(fd, buff, Consts::PAGE_SIZE);
    if(len <= 0) {
        if(len == -1 && errno != Consts::BAD_FD_ERRNO) {
            // 没有遇到过这种情况..
            log->warn("len == -1");
        }
        manager->removeById(this, ctx->consumerId);
        return;
    }

    log->info("recv proxy MESSAGE, len = %d; send MESSAGE. outerFd = %d", len, fd);
    FeedUtils::createMessage(action, fd, buff, len);
    FeedUtils::sendAction(action, ctx->pipeFd);
}

void OuterRequestHandleTask::destructEvent(ConnectPool *manager) {
    log->info("recv proxy DISCONNECT. send DISCONNECT. outerFd = %d", fd);
    auto action = FeedUtils::createDisconnect(fd);
    FeedUtils::sendAction(action, ctx->pipeFd);
    ctx->fdExists.erase(fd);
    ::close(fd);
}

void OuterRequestHandleTask::forceDestructEvent(ConnectPool *manager) {
    log->info("force proxy DISCONNECT. outerFd = %d", fd);
    ctx->fdExists.erase(fd);
    ::close(fd);
}