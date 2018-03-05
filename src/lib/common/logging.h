#pragma once
#include <sys/time.h>
#include "lib/common/base.h"

namespace qwb {
    enum LogLevel {
        DEBUG, INFO, WARN, ERROR, FATAL
    };

    class Logger {
    public:
        Logger(const char* name, int fd) {
            this->name = name;
            this->fd = fd;
        }
        void setLevel(LogLevel level) {
            level = std::max(std::min(level, FATAL), DEBUG);
            this->level = level;
        }
        void setBufSize(int bufSize) {
            if(bufSize > 1024) {
                this->bufSize = bufSize;
            }
        }

        void log(int level, const char *fmt, ...);
#define debug(fmt, ...) log(LogLevel::DEBUG, fmt, __VA_ARGS__)
#define info(fmt, ...) log(LogLevel::INFO, fmt, __VA_ARGS__)
#define warn(fmt, ...) log(LogLevel::WARN, fmt, __VA_ARGS__)
#define error(fmt, ...) log(LogLevel::ERROR, fmt, __VA_ARGS__)
#define fatal(fmt, ...) log(LogLevel::FATAL, fmt, __VA_ARGS__)

    private:
        int fd;
        int bufSize = 4096;
        const char* name;

        LogLevel level = LogLevel::ERROR;
        const char *level_name[FATAL + 1] = {"DEBUG", "INFO", "WARN", "ERROE", "FATAL"};

        Logger(const Logger&) = delete;
        Logger operator=(const Logger&) = delete;
    };

    typedef std::unique_ptr<Logger> LoggerPtr;

    class LoggerFactory {
    public:
        static LoggerPtr createToStdout(const char *name = nullptr);
        static LoggerPtr createToStderr(const char *name = nullptr);
        static LoggerPtr createToFile(const char *file_name, const char *name = nullptr);
    };
}