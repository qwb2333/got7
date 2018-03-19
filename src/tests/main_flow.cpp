#include <thread>
#include "model/feed.pb.h"
#include "lib/connect/tcp.h"
#include "lib/common/exit.h"
#include "lib/common/utils.h"
using namespace qwb;

const uint16_t proxyPort = 1080;

const uint16_t innerMainPort = 3001;

const uint16_t outerMainPort = 4001;
const uint16_t outerHandlePort = 4002;

const int buffSize = 8200;
const int dataMaxSize = 4080;

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
    static idl::FeedAction createDisconnect(int fd) {
        idl::FeedAction action;
        action.set_fd(fd);
        action.set_option(idl::FeedOption::DISCONNECT);
        return action;
    }
    static void createMessage(idl::FeedAction &action, int fd, const u_char *buff, int size) {
        action.set_fd(fd);
        action.set_data(buff, size);
        action.set_option(idl::FeedOption::MESSAGE);
    }
    static idl::FeedAction getFeedAction(const u_char *buff, int size) {
        idl::FeedAction action;
        action.ParseFromArray(buff, size);
        return action;
    }
    static void sendMessage(int outerFd, int pipeFd, u_char *buff, int size) {
        idl::FeedAction action;
        createMessage(action, outerFd, buff, size);
        int byteSize = action.ByteSize();

        // 先发一个2字节的整数,表示protobuf的长度
        u_char *ptr = buff, *limit = buff + size;
        Utils::uint16ToUChars((uint16_t)byteSize, ptr); ptr += 2;
        action.SerializeToArray(ptr, (int)(limit - ptr)); ptr += byteSize;
        ::write(pipeFd, buff, ptr - buff);
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
        log->setName("InnerMainHandle");

        tcp->socket();
        tcp->bind();
        tcp->connect("127.0.0.1", outerMainPort);
        // 让OuterMainService获得pipe用的fd
        int pipeFd = tcp->get_fd();
        FeedUtils::createPipe().SerializeToFileDescriptor(pipeFd);
        log->info("send PIPE.");

        func_main_handle(pipeFd);
    }

private:
    TcpPtr tcp;
    void func_main_handle(int pipeFd) {
        RemoteInfo remoteInfo;

        std::map<int, int> fdMap;
        u_char buff[buffSize];

        while(true) {
            int len = (int)::read(pipeFd, buff, buffSize);
            if(len == -1) {
                log->error("read fd = %d; %d; %s", pipeFd, errno, strerror(errno));
            } else if(len == 0) {
                log->info("pipe DISCONNECT.");
                break;
            }

            idl::FeedAction action = FeedUtils::getFeedAction(buff, buffSize);
            idl::FeedOption option = action.option();
            int outerFd = action.fd();

            if(option == idl::FeedOption::CONNECT) {
                log->info("recv CONNECT.");

                TcpPtr client = std::make_shared<Tcp>("127.0.0.1");
                client->setLogLevel(LogLevel::ERROR);
                client->socket();
                client->connect("127.0.0.1", proxyPort);
                int innerFd = client->get_fd();

                log->info("add map, innerFd = %d, outerFd = %d", innerFd, outerFd);
                fdMap[outerFd] = innerFd;

                std::thread thInnerHandle([this, innerFd, outerFd, pipeFd, &fdMap](){
                    log->setName("InnerHandle");
                    this->func_handle(innerFd, outerFd, pipeFd, fdMap);
                });
                thInnerHandle.detach();
            } else if(option == idl::FeedOption::DISCONNECT) {
                log->info("recv DISCONNECT.");
                if(!fdMap.count(outerFd)) {
                    log->info("had DISCONNECT.");
                } else {
                    int innerFd = fdMap[outerFd];
                    ::close(innerFd);
                    fdMap.erase(outerFd);
                }
            } else if(option == idl::FeedOption::MESSAGE) {
                log->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
                // 收到Message的消息, 直接写到proty_port里去
                if(!fdMap[outerFd]) {
                    log->info("had DISCONNECT.");
                } else {
                    int innerFd = fdMap[outerFd];
                    ::write(innerFd, action.data().c_str(), action.data().length());
                }
            }
        }
    }
    void func_handle(int innerFd, int outerFd, int pipeFd, std::map<int, int> &fdMap) {
        u_char buff[buffSize];

        while(true) {
            int len = (int)::read(innerFd, buff, buffSize);
            if(len == -1) {
                log->error("read fd = %d; %d; %s", innerFd, errno, strerror(errno));
            }else if(len == 0) {
                log->info("recv proxy DISCONNECT. send DISCONNECT.");
                FeedUtils::createDisconnect(outerFd).SerializeToFileDescriptor(pipeFd);
                fdMap.erase(outerFd);
                return;
            } else {
                log->info("recv proxy MESSAGE, len = %d; send MESSAGE.", len);
                FeedUtils::sendMessage(outerFd, pipeFd, buff, len);
            }
        }
    }
};

class OuterMainService {
public:
    OuterMainService() {
        tcpMain = std::make_shared<Tcp>("127.0.0.1", outerMainPort);
        tcpPart = std::make_shared<Tcp>("127.0.0.1", outerHandlePort);
        tcpMain->setLogLevel(LogLevel::ERROR);
        tcpPart->setLogLevel(LogLevel::ERROR);
    }
    ~OuterMainService() {
        tcpMain->close();
        tcpPart->close();
    }

    void run() {
        log->setName("OuterMainHandle");

        tcpMain->socket();
        tcpMain->bind();
        tcpMain->listen();
        log->info("wait accept.");

        RemoteInfo remoteInfo;

        // 只为了建立pipeFd
        tcpMain->accept(remoteInfo);
        int pipeFd = remoteInfo.fd;
        log->info("get PIPE. fd = %d", pipeFd);

        func_main_handle(pipeFd, tcpPart);
    }
private:
    TcpPtr tcpMain, tcpPart;
    void func_main_handle(int pipeFd, TcpPtr &tcpPart) {
        std::set<int> fdExists;
        u_char buff[buffSize];

        std::thread thOuterPartHandle([this, pipeFd, &tcpPart, &fdExists](){
            log->setName("OuterPartHandle");

            tcpPart->socket();
            tcpPart->bind();
            tcpPart->listen();

            RemoteInfo remoteInfo;
            while(true) {
                if(!tcpPart->accept(remoteInfo)) {
                    log->info("part DISCONNECT.");
                    break;
                }
                log->info("new accept, fd = %d, %s:%d", remoteInfo.fd, remoteInfo.ip.c_str(), (int)remoteInfo.port);
                int outerFd = remoteInfo.fd;

                fdExists.insert(outerFd);
                FeedUtils::createConnect(outerFd).SerializeToFileDescriptor(pipeFd);
                std::thread thOuterHandle([this, outerFd, pipeFd, &fdExists](){
                    log->setName("OuterHandle");
                    this->func_handle(outerFd, pipeFd, fdExists);
                });
                thOuterHandle.detach();
            }
        });
        thOuterPartHandle.detach();

        while(true) {
            int len = (int)::read(pipeFd, buff, buffSize);
            if(len == -1) {
                log->error("read fd = %d; %d; %s", pipeFd, errno, strerror(errno));
            } else if(len == 0) {
                log->info("pipe DISCONNECT.");
                break;
            }

            idl::FeedAction action = FeedUtils::getFeedAction(buff, buffSize);
            idl::FeedOption option = action.option();
            int outerFd = action.fd();

            if(option == idl::FeedOption::DISCONNECT) {
                log->info("recv DISCONNECT.");
                if(!fdExists.count(outerFd)) {
                    log->info("had DISCONNECT.");
                } else {
                    ::close(outerFd);
                    fdExists.erase(outerFd);
                }
            } else if(option == idl::FeedOption::MESSAGE) {
                log->info("recv MESSAGE, outerFd = %d, len = %d", outerFd, action.data().length());
                // 收到Message的消息, 直接写到proty_port里去
                if(!fdExists.count(outerFd)) {
                    log->info("had DISCONNECT.");
                } else {
                    ::write(outerFd, action.data().c_str(), action.data().length());
                }
            }
        }
    }
    void func_handle(int outerFd, int pipeFd, std::set<int> &fdExists) {
        u_char buff[buffSize];

        while(true) {
            int len = (int)::read(outerFd, buff, buffSize);
            if(len == -1) {
                log->error("read fd = %d; %d; %s", outerFd, errno, strerror(errno));
            }else if(len == 0) {
                log->info("recv proxy DISCONNECT. send DISCONNECT.");
                FeedUtils::createDisconnect(outerFd).SerializeToFileDescriptor(pipeFd);
                fdExists.erase(outerFd);
                return;
            } else {
                log->info("recv proxy MESSAGE, len = %d; send MESSAGE.", len);
                FeedUtils::sendMessage(outerFd, pipeFd, buff, len);
            }
        }
    }
};


int main() {
    std::unique_ptr<InnerMainService> innerMainService(new InnerMainService());
    std::unique_ptr<OuterMainService> outerMainService(new OuterMainService());
    saveExit(SIGINT, [&](){
        innerMainService.reset(nullptr);
        outerMainService.reset(nullptr);
        exit(0);
    });

    std::thread thOuterMainService([&](){
        outerMainService->run();
    });

    sleep(1);
    std::thread thInnerMainService([&](){
        innerMainService->run();
    });

    thOuterMainService.join();
    thInnerMainService.join();
    return 0;
}