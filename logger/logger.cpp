#include "logger.h"

namespace logger {

logger::logger(const char* ident) {
    openlog(ident, LOG_PID | LOG_NDELAY, LOG_USER);
}

};
