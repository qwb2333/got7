#include "logging.h"
using namespace qwb;

void Logger::log(int level, const char *fmt, ...) {
    if(level < this->level) {
        return;
    }

    char buffer[bufSize];
    char *p = buffer, *limit = buffer + sizeof(buffer);

    struct timeval now_tv;
    gettimeofday(&now_tv, NULL);
    const time_t seconds = now_tv.tv_sec;

    struct tm t;
    localtime_r(&seconds, &t);

    p += snprintf(
            p, limit - p, "[%s] [%04d-%02d-%02d %02d:%02d:%02d.%06d] - [%s]: ",
            level_name[level], t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
            static_cast<int>(now_tv.tv_usec), this->name
    );

    va_list args;
    va_start(args, fmt);
    p += vsnprintf(p, limit - p, fmt, args);
    va_end(args);

    p = std::min(p, limit - 2);
    while(*--p == '\n');
    *++p = '\n'; *++p = 0;

    ssize_t len = write(this->fd, buffer, p - buffer);
    if(len != p - buffer) {
        fprintf(stderr, "write log failed. written %d errmsg: %s\n", errno, strerror(errno));
    }
}

void Logger::debug(const char *fmt, ...) {
    va_list args;
    log(DEBUG, fmt, args);
}

void Logger::warn(const char *fmt, ...) {
    va_list args;
    log(WARN, fmt, args);
}

void Logger::info(const char *fmt, ...) {
    va_list args;
    log(INFO, fmt, args);
}

void Logger::error(const char *fmt, ...) {
    va_list args;
    log(ERROR, fmt, args);
}

void Logger::fatal(const char *fmt, ...) {
    va_list args;
    log(FATAL, fmt, args);
}

LoggerPtr LoggerFactory::createToStdout(const char *name) {
    if(name == nullptr) {
        name = "Main";
    }
    return LoggerPtr(new Logger(name, STDOUT_FILENO));
}

LoggerPtr LoggerFactory::createToStderr(const char *name) {
    if(name == nullptr) {
        name = "Main";
    }
    return LoggerPtr(new Logger(name, STDERR_FILENO));
}

LoggerPtr LoggerFactory::createToFile(const char *file_name, const char *name) {
    if(name == nullptr) {
        name = "Main";
    }
    int fd = open(file_name, O_WRONLY|O_APPEND|O_CREAT|O_CLOEXEC);
    if(fd < 0) {
        fprintf(stderr, "[LoggerFactory::createToFile]open %s failed.", file_name);
    }
    return LoggerPtr(new Logger(name, fd));
}