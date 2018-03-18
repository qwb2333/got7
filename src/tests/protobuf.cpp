#include <cstdio>
#include "model/feed.pb.h"
using namespace idl;

int main() {
    FeedAction feedAction;

    feedAction.set_fd(23333);
    feedAction.set_option(FeedOption::CONNECT);
    feedAction.set_data("ceshi");

    std::string str = feedAction.SerializeAsString();
    std::printf("%s\n", str.c_str());
    return 0;
}