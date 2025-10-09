#pragma once

#include <syslog.h>
#include <fmt/format.h>

namespace logger {

class logger {
public:
    logger(const char* ident);
    ~logger() {
        closelog();
    }

    template<typename... Args>
    void log(int priority, const char* format, Args&&... args) const {
        auto message = fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
        syslog(priority, "%s", message.c_str());
    }

    template<typename... Args>
    void error(const char* format, Args&&... args) {
        log(LOG_ERR, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(const char* format, Args&&... args) {
        log(LOG_WARNING, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void notice(const char* format, Args&&... args) {
        log(LOG_NOTICE, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const char* format, Args&&... args) {
        log(LOG_INFO, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(const char* format, Args&&... args) {
        log(LOG_DEBUG, format, std::forward<Args>(args)...);
    }

    static logger& get(const char* ident) {
        static logger log(ident);
        return log;
    }
};

};
