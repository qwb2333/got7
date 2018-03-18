#include "tcp.h"
using namespace qwb;

void TcpBase::createSockAddr(sockaddr_in &addr_in, const char *ip, uint16_t port) {
    inet_pton(AF_INET, ip, &addr_in.sin_addr);
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);
}

bool TcpBase::socket() {
    int result = ::socket(AF_INET, SOCK_STREAM, 0);
    if(result < 0) {
        log->error("create socket error, %d %s", errno, strerror(errno));
        return false;
    }

    fd = result;
    log->info("create socket success, fd: %d", fd);
    return true;
}

bool TcpBase::bind() {
    // 如果端口为0，则不bind
    if(!local_addr.sin_port) {
        return true;
    }

    int result = ::bind(fd, (sockaddr*)&local_addr, sizeof(local_addr));
    if(result < 0) {
        log->error("build socket error, %d %s", errno, strerror(errno));
        return false;
    }

    log->info("bind success.");
    return true;
}

void TcpBase::close() {
    ::close(fd);
}

bool TcpServer::accept(RemoteInfo &info) {
    int connect_fd = ::accept(fd, (sockaddr*)NULL, NULL);
    if(connect_fd < 0) {
        log->error("accept socket error, %d %s", errno, strerror(errno));
        return false;
    }

    sockaddr_in addr_in;
    socklen_t addr_in_len = sizeof(addr_in);
    getpeername(connect_fd, (sockaddr*) &addr_in, &addr_in_len);

    info.fd = connect_fd;
    info.ip = std::string(inet_ntoa(addr_in.sin_addr));
    info.port = ntohs(addr_in.sin_port);
    return true;
}

bool TcpServer::listen(int maxCount) {
    int result = ::listen(get_fd(), maxCount);
    if(result < 0) {
        log->error("listen socket error, %d %s", errno, strerror(errno));
        return false;
    }
    log->info("listen success.");
    return true;
}

bool TcpClient::connect(const char *ip, uint16_t port) {
    createSockAddr(remote_addr, ip, port);
    int result = ::connect(fd, (sockaddr*)&remote_addr, sizeof(remote_addr));
    if(result < 0) {
        log->error("create socket error, %d %s", errno, strerror(errno));
        return false;
    }
    return true;
}