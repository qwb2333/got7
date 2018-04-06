#include "pipe_task.h"
using namespace got7;

InnerPipeHandleTask::InnerPipeHandleTask(InnerCtx *ctx) {
    this->ctx = ctx;
    this->fd = ctx->pipeFd;
}

void InnerPipeHandleTask::readEvent(EpollRun *manager) {
    std::vector<idl::FeedAction> actionVec;
    int result = FeedUtils::readMessage(actionVec, ctx);
    if(result <= 0) {
        if(result == -1) {
            LOG->error("read fd = %d, error = %d, %s", ctx->pipeFd, errno, strerror(errno));
        }
        LOG->info("recv pipe DISCONNECT. pipeFd = %d", fd);
        manager->remove(this);
        return;
    }

    for(auto &action : actionVec) {
        int outerFd = action.fd();
        idl::FeedOption option = action.option();

        if(option == idl::FeedOption::CONNECT) {
            auto info = action.remoteinfo();
            TcpPtr client = std::make_shared<Tcp>("127.0.0.1");
            client->setLogLevel(LogLevel::ERROR);

            client->socket();
            LOG->info("recv CONNECT. to %s:%d, outerFd = %d", info.ip().c_str(), info.port(), outerFd);

            int innerFd = client->get_fd();
            if(!client->connect(info.ip().c_str(), (uint16_t)info.port())) {
                LOG->info("connect %s:%d failed.", info.ip().c_str(), info.port());
                ::close(innerFd);
                continue;
            }

            TaskBase *innerRequestHandle = new InnerRequestHandleTask(ctx, innerFd, outerFd);
            manager->add(innerRequestHandle, TaskEvents::ReadEvent);

        } else if(option == idl::FeedOption::DISCONNECT) {
            LOG->info("recv DISCONNECT. outerFd = %d", action.fd());
            auto kv = ctx->fdMap.find(outerFd);
            if(kv == ctx->fdMap.end()) {
                LOG->info("had DISCONNECT. outerFd = %d", action.fd());
            } else {
                manager->remove(kv->second.second);
            }

        } else if(option == idl::FeedOption::MESSAGE) {
            LOG->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
            // 收到Message的消息, 直接写到proxy_port里去
            auto kv = ctx->fdMap.find(outerFd);
            if(kv == ctx->fdMap.end()) {
                LOG->info("had DISCONNECT. outerFd = %d", action.fd());
            } else {
                int innerFd = kv->second.first;
                int len = (int)Tcp::write(innerFd, (void*)action.data().c_str(), (unsigned)action.data().length());
                if(len <= 0) {
                    // 这个时候outerFd可能已经关闭了,直接删掉
                    LOG->info("write failed. outerFd = %d", outerFd);
                    manager->remove(kv->second.second);
                }
            }
        } else if(option == idl::FeedOption::ACK) {
            int consumerId = action.fd();
            LOG->info("recv ACK, consumerId = %d", consumerId);
        }
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
    ::close(fd);
}
