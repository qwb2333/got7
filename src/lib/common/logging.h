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
            this->fd = fd;
            setName(name);
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

        void debug(const char *fmt, ...);
        void info(const char *fmt, ...);
        void warn(const char *fmt, ...);
        void error(const char *fmt, ...);
        void fatal(const char *fmt, ...);

        void setName(const char* name);
    private:
        int fd;
        int bufSize = 4096;
        const char* name;
        std::string string_name;

        LogLevel level = LogLevel::INFO;
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

    extern thread_local LoggerPtr log;
}