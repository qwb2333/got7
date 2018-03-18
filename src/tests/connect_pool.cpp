#include <cstdio>
#include <thread>
#include "lib/common/exit.h"
#include "lib/common/config.h"
#include "lib/connect/task.h"
#include "lib/connect/connect_pool.h"
using namespace qwb;

class TaskTest: public TaskBase {
public:
    void dealReadEvent(ConnectPool *manager) override {

        for(int i = 0; i < 64; i++) {
            char buff[8];
            int len = (int)read(this->fd, buff, 7);
            if(len == -1) break;

            char *end = buff + len;
            while(buff < end && (*end == '\n' || *end == '\0')) end--;
            *++end = '\0';

            std::printf("name: %s, [%s]\n", name.c_str(), buff);
        }
    }

    TaskTest() = default;
    TaskTest(const int fd, std::string name = "TaskTest") {
        this->fd = fd;
        this->name = name;

        log->info("TaskTest init.");
    }

private:
    std::string name;
};

int main() {
    qwb::log->setLevel(LogLevel::DEBUG);
    ConfigReaderPtr config = ConfigReaderFactory::createFromFile("/home/qwb/code/Clion/got7/config/inner_service.properties");

    int epoll_size = config->getAsInt("epoll_size", 3);
    int thread_size = config->getAsInt("thread_size", 2);
    int mainfd_size = config->getAsInt("mainfd_size", 2000);

    int fd[5][2];
    std::thread thread[5];
    ConnectPollPtr pool(new ConnectPool(thread_size, epoll_size));

    for(int i = 0; i < 5; i++) {
        qwb::log->debug("pipe = %d", pipe(fd[i]));
        qwb::log->debug("fd[0] = %d, fd[1] = %d", fd[i][0], fd[i][1]);

        char nameBuff[128];
        sprintf(nameBuff, "TaskTest-%d", i);

        TaskPtr s(new TaskTest(fd[i][0], nameBuff));
        pool->add(s, TaskEvents::ReadEvent);

        thread[i] = std::thread([i, fd]{
            int f = fd[i][1];
            char buff[1024];

            for(int j = 0; j < 10; j++) {
                sprintf(buff, "123456789,i = %d,123456789\n", j);
                write(f, buff, strlen(buff));
                sleep(1);
            }
            close(f);
        });
    }

    saveExit(SIGINT, [fd]{
        qwb::log->info("save exit.");
        exit(0);
    });

    pool->join();
    return 0;
}