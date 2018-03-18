#include <thread>
#include "lib/common/exit.h"
#include "lib/connect/tcp.h"
using namespace qwb;

int main() {
    TcpServerPtr server(new TcpServer("0.0.0.0", 4928));
    TcpClientPtr client(new TcpClient());

    saveExit(SIGINT, [&]{
        server->close();
        client->close();
    });

    std::thread th_server([&server](){
        log->setName("Server");

        server->socket();
        server->bind();
        server->listen();

        log->info("server run success.");

        RemoteInfo info;
        server->accept(info);

        int fd = info.fd;
        log->info("new client, fd: %d, %s:%d", fd, info.ip.c_str(), info.port);

        const int buffSize = 1024;
        char buff[buffSize];

        while(true) {
            int len = (int)::read(fd, buff, (unsigned)buffSize);
            if(len == 0) break;

            buff[len + 1] = '\0';
            std::printf("%s", buff);
        }
        log->info("client has been closed.");
    });

    // 保证server已经启动
    sleep(1);

    std::thread th_client([&client](){
        log->setName("Client");

        client->socket();
        client->connect("127.0.0.1", 4928);

        int fd = client->get_fd();

        const int buffSize = 1024;
        char buff[buffSize];

        for(int i = 0; i < 10; i++) {
            sprintf(buff, "i = %d\n", i);
            int len = (int)::write(fd, buff, strlen(buff));
            if(len <= 0) {
                log->info("client error.");
                break;
            }
        }
        client->close();
        log->info("client closed.");
    });

    th_server.join();
    th_client.join();
    return 0;
}