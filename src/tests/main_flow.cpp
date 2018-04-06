#include <thread>
#include "got7/protobuf/feed.pb.h"
#include "lib/connect/tcp.h"
#include "lib/common/sig.h"
#include "lib/common/utils.h"
using namespace qwb;

const uint16_t proxyPort = 443;
const char* proxyIp = "97.64.35.146";

const uint16_t innerMainPort = 3001;

const uint16_t outerMainPort = 4001;
const uint16_t outerHandlePort = 4002;

const int pageSize = 4096;
const int buffSize = 10000;
const int protoLenSize = 2;
const int badFdErrno = 9;

struct Ctx {
    int pipeFd;
    u_char buff[buffSize];
    uint16_t usedCount;
    int protoSize;

    Ctx(int pipeFd = 0):pipeFd(pipeFd){}
};
typedef std::shared_ptr<Ctx> CtxPtr;

class FeedUtils {
public:
    static idl::FeedAction createPipe() {
        idl::FeedAction action;
        action.set_option(idl::FeedOption::PIPE);
        return action;
    }
    static idl::FeedAction createConnect(int fd) {
        idl::FeedAction action;
        action.set_fd(fd);
        action.set_option(idl::FeedOption::CONNECT);
        return action;
    }
    static idl::FeedAction createAck(int consumerId) {
        idl::FeedAction action;
        action.set_fd(consumerId);
        action.set_option(idl::FeedOption::ACK);
        return action;
    }
    static idl::FeedAction createDisconnect(int fd) {
        idl::FeedAction action;
        action.set_fd(fd);
        action.set_option(idl::FeedOption::DISCONNECT);
        return action;
    }
    static void createMessage(idl::FeedAction &action, int fd, const u_char *buff, int size) {
        action.set_fd(fd);
        action.set_data(buff, (unsigned)size);
        action.set_option(idl::FeedOption::MESSAGE);
    }
    static void serializeFeedAction(idl::FeedAction &action, const u_char *buff, int size) {
        action.ParseFromArray(buff, size);
    }
    static void sendAction(idl::FeedAction &action, int pipeFd) {
        u_char buff[buffSize];
        int byteSize = action.ByteSize();

        // 先发一个2字节的整数,表示protobuf的长度
        u_char *ptr = buff, *limit = buff + buffSize;
        Utils::uint16ToUChars((uint16_t)byteSize, ptr); ptr += protoLenSize;
        action.SerializeToArray(ptr, (int)(limit - ptr)); ptr += byteSize;
        ::write(pipeFd, buff, ptr - buff);
    }
    static int readMessage(idl::FeedAction &refAction, CtxPtr &ctx) {
        if(!ctx->protoSize) {
            if(ctx->usedCount >= protoLenSize) {
                // buff中是有protoSize大小的,弄一下
                ctx->protoSize = Utils::uCharsToUint16(ctx->buff);
            } else {
                int len = (int)::read(ctx->pipeFd, ctx->buff + ctx->usedCount, (unsigned)(buffSize - ctx->usedCount));
                if(len <= 0) return len;
                ctx->usedCount += len;

                if(ctx->usedCount < protoLenSize) {
                    // 这个情况非常奇葩, 应该不会出现才对
                    LOG->error("can't get protoSize, pipeFd = %d.", ctx->pipeFd);
                    return 0; // 认为需要断开
                }
                ctx->protoSize = Utils::uCharsToUint16(ctx->buff);
            }
            if(ctx->protoSize > pageSize + 128) {
                // 这个情况理论是不存在的, protoSize理论只会稍微大于pageSize,因为protobuf中除了data还带有一些其他数据
                LOG->error("usedCount > pageSize. pipeFd = %d.", ctx->pipeFd);
                return 0; // 认为需要断开
            }
        }

        // proto内容还不足够,循环读
        int whileCount = 0;
        while(ctx->usedCount - protoLenSize < ctx->protoSize) {
            if(whileCount >= 1) {
                // 这个情况理论是不存在的
                LOG->error("whileCount > 1. pipeFd = %d", ctx->pipeFd);
                return 0;
            }
            int len = (int)::read(ctx->pipeFd, ctx->buff + ctx->usedCount, (unsigned)(buffSize - ctx->usedCount));
            if(len <= 0) return len;

            ctx->usedCount += len;
            whileCount++;
        }

        serializeFeedAction(refAction, ctx->buff + protoLenSize, ctx->protoSize);
        int resLen = ctx->usedCount - protoLenSize - ctx->protoSize;
        int beginPos = ctx->protoSize + protoLenSize;

        for(int i = 0; i < resLen; i++) {
            ctx->buff[i] = ctx->buff[i + beginPos];
        }
        ctx->protoSize = 0;
        ctx->usedCount = (uint16_t)resLen;
        return 1; // 表示正常返回
    }
};


class InnerMainService {
public:
    InnerMainService() {
        tcp = std::make_shared<Tcp>("127.0.0.1", innerMainPort);
        tcp->setLogLevel(LogLevel::ERROR);
    }
    ~InnerMainService() {
        tcp->close();
    }
    void run() {
        LOG->setName("InnerMainHandle");

        tcp->socket();
        tcp->bind();
        tcp->connect("127.0.0.1", outerMainPort);
        // 让OuterMainService获得pipe用的fd

        CtxPtr ctx = std::make_shared<Ctx>(tcp->get_fd());

        idl::FeedAction action = FeedUtils::createPipe();
        FeedUtils::sendAction(action, ctx->pipeFd);
        LOG->info("send PIPE.");

        func_main_handle(ctx);
    }

private:
    TcpPtr tcp;
    void func_main_handle(CtxPtr &ctx) {
        RemoteInfo remoteInfo;
        idl::FeedAction action;

        std::map<int, int> fdMap;
        int pipeFd = ctx->pipeFd;
        while(true) {
            int len = FeedUtils::readMessage(action, ctx);
            if(len == -1) {
                LOG->error("read fd = %d; %d; %s", pipeFd, errno, strerror(errno));
            } else if(len == 0) {
                LOG->info("pipe DISCONNECT.");
                ::close(pipeFd);
                break;
            }

            idl::FeedOption option = action.option();
            int outerFd = action.fd();

            if(option == idl::FeedOption::CONNECT) {
                LOG->info("recv CONNECT.");

                TcpPtr client = std::make_shared<Tcp>("127.0.0.1");
                client->setLogLevel(LogLevel::ERROR);
                client->socket();
                client->connect(proxyIp, proxyPort);
                int innerFd = client->get_fd();

                LOG->info("add map, innerFd = %d, outerFd = %d", innerFd, outerFd);
                fdMap[outerFd] = innerFd;

                std::thread thInnerHandle([this, innerFd, outerFd, pipeFd, &fdMap](){
                    LOG->setName("InnerHandle");
                    this->func_handle(innerFd, outerFd, pipeFd, fdMap);
                });
                thInnerHandle.detach();
            } else if(option == idl::FeedOption::DISCONNECT) {
                LOG->info("recv DISCONNECT.");
                if(!fdMap.count(outerFd)) {
                    LOG->info("had DISCONNECT.");
                } else {
                    int innerFd = fdMap[outerFd];
                    ::close(innerFd);
                    fdMap.erase(outerFd);
                }
            } else if(option == idl::FeedOption::MESSAGE) {
                LOG->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
                // 收到Message的消息, 直接写到proty_port里去
                if(!fdMap[outerFd]) {
                    LOG->info("had DISCONNECT.");
                } else {
                    int innerFd = fdMap[outerFd];
                    ::write(innerFd, action.data().c_str(), action.data().length());
                }
            }
        }
    }
    void func_handle(int innerFd, int outerFd, int pipeFd, std::map<int, int> &fdMap) {
        u_char buff[pageSize];

        idl::FeedAction action;

        while(true) {
            int len = (int)::read(innerFd, buff, pageSize);
            if(len == -1) {
                if(errno != badFdErrno) {
                    LOG->error("read fd = %d; %d; %s", innerFd, errno, strerror(errno));
                }
                return;
            }else if(len == 0) {
                LOG->info("recv proxy DISCONNECT. send DISCONNECT.");
                action = FeedUtils::createDisconnect(outerFd);
                FeedUtils::sendAction(action, pipeFd);
                fdMap.erase(outerFd);
                ::close(innerFd);
                return;
            } else {
                LOG->info("recv proxy MESSAGE, len = %d; send MESSAGE.", len);
                FeedUtils::createMessage(action, outerFd, buff, len);
                FeedUtils::sendAction(action, pipeFd);
            }
        }
    }
};

class OuterMainService {
public:
    OuterMainService() {
        tcpMain = std::make_shared<Tcp>("127.0.0.1", outerMainPort);
        tcpPart = std::make_shared<Tcp>("0.0.0.0", outerHandlePort);
        tcpMain->setLogLevel(LogLevel::ERROR);
        tcpPart->setLogLevel(LogLevel::ERROR);
    }
    ~OuterMainService() {
        tcpMain->close();
        tcpPart->close();
    }

    void run() {
        LOG->setName("OuterMainHandle");

        tcpMain->socket();
        tcpMain->bind();
        tcpMain->listen();
        LOG->info("wait accept.");

        RemoteInfo remoteInfo;

        // 只为了建立pipeFd
        tcpMain->accept(remoteInfo);


        CtxPtr ctx = std::make_shared<Ctx>(remoteInfo.fd);
        LOG->info("get PIPE. fd = %d", ctx->pipeFd);

        func_main_handle(ctx, tcpPart);
    }
private:
    TcpPtr tcpMain, tcpPart;
    void func_main_handle(CtxPtr &ctx, TcpPtr &tcpPart) {
        std::set<int> fdExists;
        int pipeFd = ctx->pipeFd;

        std::thread thOuterPartHandle([this, pipeFd, &tcpPart, &fdExists](){
            LOG->setName("OuterPartHandle");

            tcpPart->socket();
            tcpPart->bind();
            tcpPart->listen();

            RemoteInfo remoteInfo;
            idl::FeedAction action;
            while(true) {
                if(!tcpPart->accept(remoteInfo)) {
                    LOG->info("part DISCONNECT.");
                    break;
                }
                LOG->info("new accept, fd = %d, %s:%d", remoteInfo.fd, remoteInfo.ip.c_str(), (int)remoteInfo.port);
                int outerFd = remoteInfo.fd;

                fdExists.insert(outerFd);

                action = FeedUtils::createConnect(outerFd);

                auto *info = new idl::FeedRemoteInfo;
                info->set_ip(proxyIp);
                info->set_port(proxyPort);
                action.set_allocated_remoteinfo(info);

                FeedUtils::sendAction(action, pipeFd);
                std::thread thOuterHandle([this, outerFd, pipeFd, &fdExists](){
                    LOG->setName("OuterHandle");
                    this->func_handle(outerFd, pipeFd, fdExists);
                });
                thOuterHandle.detach();
            }
        });
        thOuterPartHandle.detach();

        idl::FeedAction action;
        while(true) {
            int len = FeedUtils::readMessage(action, ctx);
            if(len == -1) {
                LOG->error("read fd = %d; %d; %s", pipeFd, errno, strerror(errno));
                ::close(pipeFd);
                break;
            } else if(len == 0) {
                LOG->info("pipe DISCONNECT.");
                ::close(pipeFd);
                break;
            }

            idl::FeedOption option = action.option();
            int outerFd = action.fd();

            if(option == idl::FeedOption::DISCONNECT) {
                LOG->info("recv DISCONNECT.");
                if(!fdExists.count(outerFd)) {
                    LOG->info("had DISCONNECT.");
                } else {
                    ::close(outerFd);
                    fdExists.erase(outerFd);
                }
            } else if(option == idl::FeedOption::MESSAGE) {
                LOG->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
                // 收到Message的消息, 直接写到proty_port里去
                if(!fdExists.count(outerFd)) {
                    LOG->info("had DISCONNECT.");
                } else {
                    ::write(outerFd, action.data().c_str(), action.data().length());
                }
            } else if(option == idl::FeedOption::PIPE) {
                int consumerId = action.fd();
                LOG->info("recv PIPE. consumerId = %d", consumerId);
                action = FeedUtils::createAck(consumerId);
                FeedUtils::sendAction(action, pipeFd);
            }
        }
    }
    void func_handle(int outerFd, int pipeFd, std::set<int> &fdExists) {
        u_char buff[pageSize];

        idl::FeedAction action;
        while(true) {
            int len = (int)::read(outerFd, buff, pageSize);
            if(len == -1) {
                if(errno != badFdErrno) {
                    LOG->error("read fd = %d; %d; %s", outerFd, errno, strerror(errno));
                }
                ::close(outerFd);
                return;
            }else if(len == 0) {
                LOG->info("recv proxy DISCONNECT. send DISCONNECT.");
                action = FeedUtils::createDisconnect(outerFd);
                FeedUtils::sendAction(action, pipeFd);
                fdExists.erase(outerFd);
                ::close(outerFd);
                return;
            } else {
                LOG->info("recv proxy MESSAGE, len = %d; send MESSAGE.", len);
                FeedUtils::createMessage(action, outerFd, buff, len);
                FeedUtils::sendAction(action, pipeFd);
            }
        }
    }
};


int main() {
    //std::unique_ptr<InnerMainService> innerMainService(new InnerMainService());
    std::unique_ptr<OuterMainService> outerMainService(new OuterMainService());
    saveExit(SIGINT, [&](){
        //innerMainService.reset(nullptr);
        outerMainService.reset(nullptr);
        exit(0);
    });

    std::thread thOuterMainService([&](){
        outerMainService->run();
    });

    /*sleep(1);
    std::thread thInnerMainService([&](){
        innerMainService->run();
    });*/

    thOuterMainService.join();
    //thInnerMainService.join();
    return 0;
}