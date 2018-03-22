#include <cstdio>
#include "got7/protobuf/feed.pb.h"
using namespace idl;

FeedAction action;
FeedRemoteInfo *info = nullptr;

void func() {
    info = new FeedRemoteInfo();
    info->set_ip("127.0.0.1");

    action.set_option(FeedOption::CONNECT);

    // action销毁的时候,会自动delete info
    action.set_allocated_remoteinfo(info);

    const int buffSize = 1024;
    char buff[buffSize];
    action.SerializeToArray(buff, buffSize);
}

int main() {
    func();
    printf("%s\n", info->ip().c_str());
    return 0;
}