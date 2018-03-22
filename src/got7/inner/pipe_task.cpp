#include "pipe_task.h"
using namespace got7;

InnerPipeHandleTask::InnerPipeHandleTask(InnerCtx *ctx, int fd) {
    this->fd = fd;
    this->ctx = ctx;
}

void InnerPipeHandleTask::readEvent(ConnectPool *manager) {
    idl::FeedAction action;
    int result = FeedUtils::readMessage(action, ctx);
    if(result <= 0) {
        if(result == -1) {
            log->error("read fd = %d, error = %d, %s", ctx->pipeFd, errno, strerror(errno));
        }
        manager->removeById(this, ctx->consumerId);
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
        log->info("add map, innerFd = %d, outerFd = %d", innerFd, outerFd);
        ctx->fdMap[outerFd] = innerFd;

        TaskBase *innerRequestHandle = new InnerRequestHandleTask(ctx, innerFd, outerFd);
        manager->addById(innerRequestHandle, TaskEvents::ReadEvent, ctx->consumerId);
    } else if(option == idl::FeedOption::DISCONNECT) {
        log->info("recv DISCONNECT. outerFd = %d", action.fd());
        if(!ctx->fdMap.count(outerFd)) {
            log->info("had DISCONNECT. outerFd = %d", action.fd());
        } else {
            int innerFd = ctx->fdMap[outerFd];
            ::close(innerFd);
            ctx->fdMap.erase(outerFd);
        }
    } else if(option == idl::FeedOption::MESSAGE) {
        log->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
        // 收到Message的消息, 直接写到proxy_port里去
        if(!ctx->fdMap[outerFd]) {
            log->info("had DISCONNECT. outerFd = %d", action.fd());
        } else {
            int innerFd = ctx->fdMap[outerFd];
            ::write(innerFd, action.data().c_str(), action.data().length());
        }
    } else if(option == idl::FeedOption::ACK) {
        log->info("recv ACK");
    }
}

void InnerPipeHandleTask::destructEvent(ConnectPool *manager) {
    // 释放epoll里所有的连接
    manager->getEpollRun(ctx->consumerId).reset();
    ctx->reset(); //清空ctx

    log->info("pipe DISCONNECT. fd = ", fd);
    ::close(fd); //所有的task在析构的时候记得关闭fd
}

void InnerPipeHandleTask::forceDestructEvent(ConnectPool *manager) {
    log->info("force, pipe DISCONNECT. fd = ", fd);
    ::close(fd);
}