#include "service.h"
using namespace got7;

bool OuterService::prepare(uint16_t outerPipePort) {
    log->setName("OuterService");

    TcpPtr tcp = std::make_shared<Tcp>("0.0.0.0", outerPipePort);
    tcp->setLogName("OuterServiceTcp");
    tcp->setLogLevel(LogLevel::WARN);

    bool success = true;
    success &= tcp->socket();
    success &= tcp->bind();
    success &= tcp->listen();
    if(!success) {
        log->error("socket, bind, listen failed. errno = %d, %s", errno, strerror(errno));
        return false;
    }

    const int usedConsumerId = acceptCount++;
    auto *outerPipeCenterManager = new OuterPipeCenterManager(outerCtxArr, tcp, &acceptCount, usedConsumerId);

    std::thread th([outerPipeCenterManager]() {
        log->setName("OuterPipeCenterManager");
        log->info("ready for get PIPE.");

        while(true) {
            outerPipeCenterManager->loopOnce();
        }
    });
    th.detach();

    return true;
}

void OuterService::realRun(EpollRun &epollRun, int consumerId) {
    log->setName(Utils::format("OuterService-%d", consumerId).c_str());

    epollRun.setLogName(Utils::format("OuterServiceEpoll-%d", consumerId).c_str());
    epollRun.setLogLevel(LogLevel::WARN);

    OuterCtx *ctx = &outerCtxArr[consumerId];
    while(true) {
        if(requireNewPipe(ctx)) {
            log->info("get pipeId success.");

            TaskBase *outerPipeHandleTask = new OuterPipeHandleTask(ctx, ctx->pipeFd);
            epollRun.add(outerPipeHandleTask, TaskEvents::ReadEvent);
        }

        epollRun.loopOnce();
    }
}

bool OuterService::requireNewPipe(OuterCtx *ctx) {
    if(!ctx->pipeFd) {
        log->info("wait for PIPE.");
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
        log->error("socket, bind, listen failed. errno = %d, %s", errno, strerror(errno));
        return false;
    }

    const int usedConsumerId = (acceptCount++) % threadSize;
    OuterCtx *ctx = &outerCtxArr[usedConsumerId];
    TaskBase *outerRequestCenterTask = new OuterRequestCenterTask(ctx, tcp, innerProxyIp, innerProxyPort);
    this->addById(outerRequestCenterTask, TaskEvents::ReadEvent, usedConsumerId);
    return true;
}