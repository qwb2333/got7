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
        LOG->info("OuterPipeCenterManager, CLOSED.");
        ::close(tcp->get_fd());
        return;
    }

    LOG->info("new accept %s:%d, fd = %d", remoteInfo.ip.c_str(), (int)remoteInfo.port, remoteInfo.fd);

    // 临时的ctx环境
    OuterCtx ctx(consumerId);
    ctx.pipeFd = remoteInfo.fd;

    Tcp::setSocketTimeout(ctx.pipeFd, 10); //设置超时

    std::vector<idl::FeedAction> actionVec;
    if(FeedUtils::readMessage(actionVec, (CtxBase*)&ctx) <= 0) {
        LOG->info("read PIPE failed.");
        ::close(ctx.pipeFd);
        return;
    }
    Tcp::setSocketTimeout(ctx.pipeFd);

    // 这里只需要判断第一个即可
    idl::FeedAction action = actionVec[0];
    auto option = action.option();
    if(option == idl::FeedOption::PIPE) {
        int consumerId = action.fd();
        int pipeFd = remoteInfo.fd;
        action = FeedUtils::createAck(consumerId);
        if(FeedUtils::sendAction(action, pipeFd) <= 0) {
            LOG->info("send ACK failed. consumerId = %d", consumerId);
            ::close(ctx.pipeFd);
            return;
        }

        OuterCtx &threadCtx = outerCtxArr[consumerId];
        std::unique_lock<std::mutex> uniqueLock(threadCtx.mutex);
        LOG->info("send ACK success.ready to update ctx.");

        // 把临时的ctx复制到对应线程的ctx,并唤醒
        threadCtx.copyFrom(&ctx);
        uniqueLock.unlock();
        threadCtx.condition.notify_one();
    } else {
        LOG->info("when accept, first message should PIPE. option = %d", (int)option);
        ::close(ctx.pipeFd);
    }
}


void OuterPipeHandleTask::readEvent(EpollRun *manager) {
    int pipeFd = ctx->pipeFd;
    std::vector<idl::FeedAction> actionVec;
    int len = FeedUtils::readMessage(actionVec, ctx);

    if(len <= 0) {
        if(len == -1) {
            LOG->error("read fd = %d, errno = %d, %s", pipeFd, errno, strerror(errno));
        }
        LOG->info("recv pipe DISCONNECT. pipeFd = %d", pipeFd);
        manager->remove(fd);
        return;
    }

    for(auto &action : actionVec) {
        int outerFd = action.fd();
        idl::FeedOption option = action.option();

        if(option == idl::FeedOption::ACK) {
            int consumerId = action.fd();
            LOG->info("recv ACK. consumerId = %d", consumerId);
            if(ctx->consumerId != consumerId) {
                LOG->warn("consumerId != this->consumerId.");
            }
            action = FeedUtils::createAck(consumerId);
            FeedUtils::sendAction(action, pipeFd);
            LOG->info("send ACK. consumerId = %d", consumerId);
        } else if(option == idl::FeedOption::PIPE) {
            // 理论这里应该是不会收到PIPE的
            LOG->warn("It shoud not get PIPE.");
        } else {
            if(!action.has_fd()) {
                LOG->warn("It should have fd. FeedOption = %d", (int)action.option());
                return;
            }

            TaskBase *task = NULL;
            {
                AutoRead lock(&ctx->rwLock);
                auto iter = ctx->fdMap.find(outerFd);
                if(iter == ctx->fdMap.end()) {
                    LOG->info("had DISCONNECT. outerFd = %d", outerFd);
                    return;
                }
                task = iter->second;
            }


            if(option == idl::FeedOption::DISCONNECT) {
                LOG->info("recv DISCONNECT. outerFd = %d", outerFd);
                manager->remove(fd);
            } else if(option == idl::FeedOption::MESSAGE) {
                LOG->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
                len = (int)Tcp::write(outerFd, (void*)action.data().c_str(), (unsigned)action.data().length());
                if(len <= 0) {
                    // 这个时候outerFd可能已经关闭了,直接删掉
                    LOG->info("write failed. outerFd = %d", outerFd);
                    manager->remove(fd);
                }
            }
        }
    }
}

void OuterPipeHandleTask::removeEvent(EpollRun *manager) {
    {
        AutoWrite lock(&ctx->rwLock);

        // 释放ctx里所有的request连接
        auto iter = ctx->fdMap.begin();
        while(iter != ctx->fdMap.end()) {
            OuterRequestHandleTask *task = (OuterRequestHandleTask*)iter->second;

            task->hadLocked = true;
            manager->remove(fd);

            iter = ctx->fdMap.begin();
        }
    }

    ctx->reset(); //清空ctx
    LOG->info("pipe DISCONNECT. pipeFd = %d", fd);
    ::close(fd);
    delete this;
}
