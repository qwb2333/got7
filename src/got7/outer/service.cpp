#include "service.h"
using namespace got7;

bool OuterService::prepare(uint16_t outerPipePort) {
    LOG->setName("OuterService");

    TcpPtr tcp = std::make_shared<Tcp>("0.0.0.0", outerPipePort);
    tcp->setLogName("OuterServiceTcp");
    tcp->setLogLevel(LogLevel::WARN);

    bool success = true;
    success &= tcp->socket();
    success &= tcp->bind();
    success &= tcp->listen();
    if(!success) {
        LOG->error("socket, bind, listen failed. errno = %d, %s", errno, strerror(errno));
        return false;
    }

    const int usedConsumerId = acceptCount++;
    auto *outerPipeCenterManager = new OuterPipeCenterManager(outerCtxArr, tcp, &acceptCount, usedConsumerId);

    std::thread th([outerPipeCenterManager]() {
        LOG->setName("OuterPipeCenterManager");
        LOG->info("ready for get PIPE.");

        while(true) {
            outerPipeCenterManager->loopOnce();
        }
    });
    th.detach();

    return true;
}

void OuterService::realRun(EpollRun &epollRun, int consumerId) {
    LOG->setName(Utils::format("OuterService-%d", consumerId).c_str());

    epollRun.setLogName(Utils::format("OuterServiceEpoll-%d", consumerId).c_str());
    epollRun.setLogLevel(LogLevel::WARN);

    TaskBase *outerPipeHandleTask = NULL;
    OuterCtx *ctx = &outerCtxArr[consumerId];
    while(true) {
        if(requireNewPipe(ctx)) {
            LOG->info("get pipeId success. consumerId = %d", consumerId);

            outerPipeHandleTask = new OuterPipeHandleTask(ctx);
            epollRun.add(outerPipeHandleTask, TaskEvents::ReadEvent);
        }

        epollRun.loopOnce();

        uint64_t nowTime = Utils::getTimeNow();
        if(ctx->pipeFd && nowTime - ctx->lastReadTime > 600) {
            // 如果过去10分钟了,还是没有一个ACK,说明这个pipeFd已经挂掉了,释放掉
            epollRun.remove(outerPipeHandleTask);
        }
    }
}

bool OuterService::requireNewPipe(OuterCtx *ctx) {
    if(!ctx->pipeFd) {
        LOG->info("wait for PIPE.");
        std::unique_lock<std::mutex> uniqueLock(ctx->mutex);
        ctx->condition.wait(uniqueLock, [&ctx]() {
            return ctx->pipeFd > 0;
        });
        return true;
    }
    return false;
}


bool OuterService::addRequestCenter(const char *innerProxyIp, uint16_t innerProxyPort, uint16_t outerProxyPort) {
    TcpPtr tcp = std::make_shared<Tcp>("0.0.0.0", outerProxyPort);
    tcp->setLogName("OuterRequestCenterTcp");
    tcp->setLogLevel(LogLevel::WARN);

    bool success = true;
    success &= tcp->socket();
    success &= tcp->bind();
    success &= tcp->listen();
    if(!success) {
        LOG->error("socket, bind, listen failed. errno = %d, %s", errno, strerror(errno));
        return false;
    }

    const int usedConsumerId = (acceptCount++) % threadSize;
    TaskBase *outerRequestCenterTask = new OuterRequestCenterTask(outerCtxArr,
            usedConsumerId, tcp, innerProxyIp, innerProxyPort);
    this->getEpollRun(usedConsumerId).add(outerRequestCenterTask, TaskEvents::ReadEvent);
    return true;
}
