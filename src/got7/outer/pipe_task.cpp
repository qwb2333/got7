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

    // 临时的ctx环境
    OuterCtx ctx(consumerId);
    ctx.pipeFd = remoteInfo.fd;

    idl::FeedAction action;
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
        action = FeedUtils::createAck(consumerId);
        if(FeedUtils::sendAction(action, pipeFd) <= 0) {
            log->info("send ACK failed. consumerId = %d", consumerId);
            ::close(ctx.pipeFd);
            return;
        }

        OuterCtx &threadCtx = outerCtxArr[consumerId];
        std::unique_lock<std::mutex> uniqueLock(threadCtx.mutex);
        log->info("send ACK success.ready to update ctx.");

        // 取消超时
        Tcp::setSocketTimeout(ctx.pipeFd);

        // 把临时的ctx复制到对应线程的ctx,并唤醒
        threadCtx.copyFrom(&ctx);
        uniqueLock.unlock();
        threadCtx.condition.notify_one();
    } else {
        log->info("when accept, first message should PIPE. option = %d", (int)option);
        ::close(ctx.pipeFd);
    }
}


void OuterPipeHandleTask::readEvent(EpollRun *manager) {
    idl::FeedAction action;

    int pipeFd = ctx->pipeFd;
    int len = FeedUtils::readMessage(action, ctx);

    if(len <= 0) {
        if(len == -1) {
            log->error("read fd = %d, errno = %d, %s", pipeFd, errno, strerror(errno));
        }
        manager->remove(this);
        return;
    }

    int outerFd = action.fd();
    idl::FeedOption option = action.option();

    if(option == idl::FeedOption::ACK) {
        int consumerId = action.fd();
        log->info("recv ACK. consumerId = %d", consumerId);
        if(ctx->consumerId != consumerId) {
            log->warn("consumerId != this->consumerId.");
        }
        action = FeedUtils::createAck(consumerId);
        FeedUtils::sendAction(action, pipeFd);
        log->info("send ACK. consumerId = %d", consumerId);
    } else if(option == idl::FeedOption::PIPE) {
        // 理论这里应该是不会收到PIPE的
        log->warn("It shoud not get PIPE.");
    } else {
        if(!action.has_fd()) {
            log->warn("It should have fd.");
            return;
        }

        TaskBase *task = NULL;
        {
            AutoRead lock(&ctx->rwLock);
            auto iter = ctx->fdMap.find(outerFd);
            if(iter == ctx->fdMap.end()) {
                log->info("had DISCONNECT. outerFd = %d", outerFd);
                return;
            }
            task = iter->second;
        }


        if(option == idl::FeedOption::DISCONNECT) {
            log->info("recv DISCONNECT. outerFd = %d", outerFd);
            manager->remove(task);
        } else if(option == idl::FeedOption::MESSAGE) {
            log->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
            ::write(outerFd, action.data().c_str(), action.data().length());
        }
    }
}

void OuterPipeHandleTask::destructEvent(EpollRun *manager) {
    {
        AutoWrite lock(&ctx->rwLock);

        // 释放ctx里所有的request连接
        auto iter = ctx->fdMap.begin();
        while(iter != ctx->fdMap.end()) {
            TaskBase *task = iter->second;
            manager->remove(task);

            iter = ctx->fdMap.begin();
        }
    }

    ctx->reset(); //清空ctx
    log->info("pipe DISCONNECT. pipeFd = %d", fd);
    ::close(fd);
}