#include "service.h"
using namespace got7;

void InnerService::setOuterServiceInfo(std::string outerIp, uint16_t outerPipePort) {
    this->outerIp = outerIp;
    this->outerPipePort = outerPipePort;
}

void InnerService::realRun(EpollRun &epollRun, int consumerId) {
    LOG->setName(Utils::format("InnerService-%d", consumerId).c_str());

    epollRun.setLogName(Utils::format("InnerServiceEpoll-%d", consumerId).c_str());
    epollRun.setLogLevel(LogLevel::WARN);

    TcpPtr tcp = std::make_shared<Tcp>("127.0.0.1");

    InnerCtx *ctx = &innerCtxArr[consumerId];
    tcp->setLogName(Utils::format("InnerServiceTcp-%d", consumerId).c_str());
    tcp->setLogLevel(LogLevel::WARN);

    TaskBase *innerPipeHandleTask = NULL;
    while(true) {
        // 会一直尝试连接,直到成功
        if(requireNewPipe(tcp, ctx, consumerId)) {
            LOG->info("create new PIPE.");
            innerPipeHandleTask = new InnerPipeHandleTask(ctx);
            epollRun.add(innerPipeHandleTask, TaskEvents::ReadEvent);
        }

        epollRun.loopOnce();

        uint64_t nowTime = Utils::getTimeNow();
        if(ctx->pipeFd && nowTime - ctx->lastReadTime > 180) {
            // 有180秒没有进行过数据的交互,发送一个ACK包过去作为心跳包
            auto action = FeedUtils::createAck(consumerId);
            if(FeedUtils::sendAction(action, ctx->pipeFd) <= 0) {
                // 这个pipdFd不行了
                LOG->warn("send ACK failed. consumerId = %d", consumerId);
                this->remove(innerPipeHandleTask);
            } else {
                LOG->info("send ACK. consumerId = %d", consumerId);
            }
        }

        if(ctx->pipeFd && nowTime - ctx->lastReadTime > 600) {
            // 如果过去10分钟了,还是没有任何数据,说明这个pipeFd已经挂掉了,释放掉
            LOG->info("lastReadTime over 600s. close innerPipeHandleTask.");
            epollRun.remove(innerPipeHandleTask);
        }
    }
}

bool InnerService::requireNewPipe(TcpPtr tcp, InnerCtx *ctx, int consumerId) {
    while(!ctx->pipeFd) {
        tcp->socket();
        int pipeFd = tcp->get_fd();

        if(!tcp->connect(outerIp.c_str(), outerPipePort)) {
            LOG->warn("connect OuterService failed. sleep 10s.");
            ::close(pipeFd); ctx->reset(); sleep(10);
            continue;
        }

        LOG->info("wait for PIPE.");

        std::vector<idl::FeedAction> actionVec;
        auto action = FeedUtils::createPipe(consumerId);

        if(FeedUtils::sendAction(action, pipeFd) <= 0) {
            LOG->warn("send PIPE failed.sleep 10s.");
            ::close(pipeFd); ctx->reset(); sleep(10);
            continue;
        }

        ctx->reset(); ctx->pipeFd = pipeFd;
        Tcp::setSocketTimeout(pipeFd, 10);

        int result = FeedUtils::readMessage(actionVec, ctx);
        if(result <= 0) {
            if(result == -1) {
                LOG->warn("readMessage error, errno = %d, %s", errno, strerror(errno));
            }
            LOG->info("readMessage failed.");
            ::close(pipeFd); ctx->reset(); sleep(10);
            continue;
        }


        action = actionVec[0];

        Tcp::setSocketTimeout(pipeFd); //取消超时

        if(action.option() == idl::FeedOption::ACK) {
            return true; // 成功连接
        }
        LOG->warn("get ACK failed.sleep 10s.");
        ::close(pipeFd); ctx->reset(); sleep(10);
    }
    return false;
}
