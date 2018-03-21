#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "lib/common/base.h"
#include "lib/common/logging.h"

namespace qwb {
    struct RemoteInfo {
        int fd;
        std::string ip;
        uint16_t port;
    };

    class Tcp {
    public:
        static void setSocketTimeout(int fd, int timeout = 0);

        bool socket();
        bool bind();
        bool connect(const char *ip, uint16_t port);
        void close();
        bool accept(RemoteInfo &info);
        bool listen(int maxCount = 1024);

        int get_fd() const {
            return fd;
        }
        void setLogLevel(LogLevel logLevel) {
            log->setLevel(logLevel);
        }
        void setLogName(const char *name) {
            log->setName(name);
        }

        Tcp():fd(-1){ }
        Tcp(const char *ip, uint16_t port = 0):fd(-1) {
            createSockAddr(local_addr, ip, port);
        }

    private:
        int fd;
        sockaddr_in local_addr, remote_addr;
        LoggerPtr log = LoggerFactory::createToStderr("Tcp");

        void createSockAddr(sockaddr_in &addr_in, const char *ip, uint16_t port);
    };

    typedef std::shared_ptr<Tcp> TcpPtr;
}