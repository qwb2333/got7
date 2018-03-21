#include "service.h"
using namespace got7;

void InnerService::setOuterServiceInfo(std::string outerIp, uint16_t outerPipePort) {
    this->outerIp = outerIp;
    this->outerPipePort = outerPipePort;
}

void InnerService::realRun(EpollRun &epollRun, int consumerId) {
    log->setName(Utils::format("InnerService-%d", consumerId).c_str());

    epollRun.setLogName(Utils::format("InnerServiceEpoll-%d", consumerId).c_str());
    epollRun.setLogLevel(LogLevel::WARN);

    TcpPtr tcp = std::make_shared<Tcp>("127.0.0.1");

    InnerCtx *ctx = &innerCtxVec[consumerId];
    tcp->setLogName(Utils::format("InnerServiceTcp-%d", consumerId).c_str());
    tcp->setLogLevel(LogLevel::WARN);

    while(true) {
        // 会一直尝试连接,直到成功
        if(requireNewPipe(tcp, ctx, consumerId)) {
            log->info("create new PIPE.");
            TaskBase *innerPipeHandle = new InnerPipeHandleTask(ctx, ctx->pipeFd);
            epollRun.add(innerPipeHandle, TaskEvents::ReadEvent);
        }

        epollRun.loopOnce();

        uint64_t nowTime = Utils::getTimeNow();
        if(nowTime - ctx->lastReadTime > 50) {
            // 有50秒没有进行过数据的交互,发送一个ACK包过去作为心跳包
            auto action = FeedUtils::createAck();
            FeedUtils::sendAction(action, ctx->pipeFd);
            log->info("send ACK.");
        }
    }
}

bool InnerService::requireNewPipe(TcpPtr tcp, InnerCtx *ctx, int consumerId) {
    while(!ctx->pipeFd) {
        tcp->socket();
        int pipeFd = tcp->get_fd();

        if(!tcp->connect(outerIp.c_str(), outerPipePort)) {
            log->warn("connect OuterService failed. sleep 10s.");
            ::close(pipeFd); ctx->reset(); sleep(10);
            continue;
        }

        auto action = FeedUtils::createPipe(consumerId);
        if(!FeedUtils::sendAction(action, pipeFd)) {
            log->warn("send PIPE failed.sleep 10s.");
            ::close(pipeFd); ctx->reset(); sleep(10);
            continue;
        }

        ctx->reset(); ctx->pipeFd = pipeFd;
        FeedUtils::readMessage(action, ctx);
        if(action.option() == idl::FeedOption::ACK) {
            return true; // 成功连接
        }
        log->warn("get ACK failed.sleep 10s.");
        ::close(pipeFd); ctx->reset(); sleep(10);
    }
    return false;
}