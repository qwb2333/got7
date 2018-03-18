#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "lib/common/base.h"
#include "lib/common/logging.h"

namespace qwb {
    class TcpBase {
    public:
        TcpBase():fd(-1){ }

        bool socket();
        bool bind();
        void close();
        int get_fd() const {
            return fd;
        }

    protected:
        int fd;
        sockaddr_in local_addr, remote_addr;
        LoggerPtr log = LoggerFactory::createToStderr("TcpBase");

        void createSockAddr(sockaddr_in &addr_in, const char *ip, uint16_t port);
    };

    struct RemoteInfo {
        int fd;
        std::string ip;
        uint16_t port;
    };

    class TcpServer: public TcpBase {
    public:
        TcpServer(const TcpServer &tcpServer) = delete;
        TcpServer operator=(const TcpServer &tcpServer) = delete;

        TcpServer(const char *ip, uint16_t port = 0) {
            createSockAddr(local_addr, ip, port);
        }

        bool accept(RemoteInfo &info);
        bool listen(int maxCount = 128);
    };
    typedef std::unique_ptr<TcpServer> TcpServerPtr;


    class TcpClient: public TcpBase {
    public:
        TcpClient() = default;
        TcpClient(const TcpClient &tcpClient) = delete;
        TcpClient operator=(const TcpClient &tcpClient) = delete;

        TcpClient(const char *ip, uint16_t port = 0) {
            createSockAddr(local_addr, ip, port);
        }

        bool connect(const char *ip, uint16_t port);
    };
    typedef std::unique_ptr<TcpClient> TcpClientPtr;
}