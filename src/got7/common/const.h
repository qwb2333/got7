#pragma once

namespace got7 {
    class Consts {
    public:
        static const int MAX_LISTEN_COUNT = 1024;
        static const int BUFF_SIZE = 10000;
        static const int PAGE_SIZE = 4096;
        static const int PROTO_LEN_SIZE = 2;

        static const int BAD_FD_ERRNO = 9;
    };
};