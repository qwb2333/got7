#pragma once
#include <sys/socket.h>
#include <netinet/tcp.h>
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
        void setNoBlock() {
            fcntl(fd, F_SETFL, O_NONBLOCK);
        }
        void setNoNagle() {
            const int noDelay = 1;
            setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,(void*)&noDelay, sizeof(int));
        }

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

        static int read(int fd, void* buff, unsigned buffSize) {
            int count = 0;
            while(count <= 100) {
                int ret = (int)::read(fd, buff, buffSize);

                if(ret >= 0) {
                    return ret;
                } else if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                    LOG->warn("read wait. sleep 0.1s. errno = %d, %s", errno, strerror(errno));
                    count++; usleep(100);
                    continue;
                }
                LOG->error("TCP::read, result = %d, errno = %d, %s", ret, errno, strerror(errno));
                return ret;
            }
            return 0;
        }

        static int write(int fd, void* buff, unsigned buffSize) {
            int count = 0;
            while(count <= 100) {
                int ret = (int)::write(fd, buff, buffSize);

                if(ret >= 0) {
                    return ret;
                } else if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                    LOG->warn("write wait. sleep 0.1s. errno = %d, %s", errno, strerror(errno));
                    count++; usleep(100);
                    continue;
                }
                LOG->error("TCP::write, result = %d, errno = %d, %s", ret, errno, strerror(errno));
                return ret;
            }
            return 0; //等待次数太多,直接断开
        }


    private:
        int fd;
        sockaddr_in local_addr, remote_addr;
        LoggerPtr log = LoggerFactory::createToStderr("Tcp");

        void createSockAddr(sockaddr_in &addr_in, const char *ip, uint16_t port);
    };

    typedef std::shared_ptr<Tcp> TcpPtr;
}