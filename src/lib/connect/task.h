#pragma once

namespace qwb {
    class TaskBase {
    public:
        int fd;
        virtual ~TaskBase() = default;

        virtual void dealReadEvent() {

        }

        virtual void dealWriteEvent() {

        }
    };
}