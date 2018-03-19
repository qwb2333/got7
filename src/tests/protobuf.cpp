#include <cstdio>
#include "model/feed.pb.h"
using namespace idl;

int main() {
    FeedAction feedAction;

    feedAction.set_fd(23333);
    feedAction.set_option(FeedOption::CONNECT);
    feedAction.set_data("ceshi");

    const int buffSize = 1024;
    char buff[buffSize];

    int len = feedAction.ByteSize();
    feedAction.SerializeToArray(buff, buffSize);

    FeedAction feedNew;
    feedNew.ParseFromArray(buff, len);
    std::printf("%d, %s\n", feedNew.fd(), feedNew.data().c_str());
    return 0;
}