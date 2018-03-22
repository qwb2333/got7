#include "pipe_task.h"
using namespace got7;

InnerPipeHandleTask::InnerPipeHandleTask(InnerCtx *ctx) {
    this->ctx = ctx;
    this->fd = ctx->pipeFd;
}

void InnerPipeHandleTask::readEvent(EpollRun *manager) {
    idl::FeedAction action;
    int result = FeedUtils::readMessage(action, ctx);
    if(result <= 0) {
        if(result == -1) {
            log->error("read fd = %d, error = %d, %s", ctx->pipeFd, errno, strerror(errno));
        }
        manager->remove(this);
        return;
    }

    int outerFd = action.fd();
    idl::FeedOption option = action.option();

    if(option == idl::FeedOption::CONNECT) {
        auto info = action.remoteinfo();
        TcpPtr client = std::make_shared<Tcp>("127.0.0.1");
        client->setLogLevel(LogLevel::ERROR);

        client->socket();
        log->info("recv CONNECT. to %s:%d, outerFd = %d", info.ip().c_str(), info.port(), outerFd);

        if(!client->connect(info.ip().c_str(), (uint16_t)info.port())) {
            log->info("connect %s:%d failed.", info.ip().c_str(), info.port());
            return;
        }

        int innerFd = client->get_fd();
        TaskBase *innerRequestHandle = new InnerRequestHandleTask(ctx, innerFd, outerFd);
        manager->add(innerRequestHandle, TaskEvents::ReadEvent);

    } else if(option == idl::FeedOption::DISCONNECT) {
        log->info("recv DISCONNECT. outerFd = %d", action.fd());
        auto kv = ctx->fdMap.find(outerFd);
        if(kv == ctx->fdMap.end()) {
            log->info("had DISCONNECT. outerFd = %d", action.fd());
        } else {
            manager->remove(kv->second.second);
        }

    } else if(option == idl::FeedOption::MESSAGE) {
        log->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
        // 收到Message的消息, 直接写到proxy_port里去
        auto kv = ctx->fdMap.find(outerFd);
        if(kv == ctx->fdMap.end()) {
            log->info("had DISCONNECT. outerFd = %d", action.fd());
        } else {
            int innerFd = kv->second.first;
            ::write(innerFd, action.data().c_str(), action.data().length());
        }
    } else if(option == idl::FeedOption::ACK) {
        int consumerId = action.fd();
        log->info("recv ACK, consumerId = %d", consumerId);
    }
}

void InnerPipeHandleTask::destructEvent(EpollRun *manager) {
    // 释放ctx里所有的request连接
    auto iter = ctx->fdMap.begin();
    while(iter != ctx->fdMap.end()) {
        TaskBase *task = iter->second.second;
        manager->remove(task);

        iter = ctx->fdMap.begin();
    }

    ctx->reset(); //清空ctx
    log->info("pipe DISCONNECT. pipeFd = %d", fd);
    ::close(fd);
}