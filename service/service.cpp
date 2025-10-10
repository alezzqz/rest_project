#include <syslog.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <filesystem>

#include "service.h"

#include "logger/logger.h"

namespace service {

extern logger::logger& log();

using namespace std;

service::service(const std::string& pid_file_path)
    : _pid_file(pid_file_path) {
}

service_start_result service::start() {
    pid_t pid = fork();
    if (pid < 0) {
        logger::logger::get("cmserver").log(LOG_INFO, "can't create new process");
        return failed;
    }

    if (pid > 0) {
        return parent;
    }

    if (setsid() < 0) {
        syslog(LOG_ERR, "can't create new session");
        return failed;
    }

    _pid_file.create(getpid());

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    umask(0);
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return started;
}

void service::stop() {
    pid_t pid = _pid_file.get_pid();
    if (pid == 0) {
        log().info("service not started");
        return;
    }

    if (kill(pid, SIGTERM) == 0) {
        log().info("stop signal sent");
        bool stopped = false;

        for (int i = 0; i < 10; ++i) {
            if (kill(pid, 0) != 0) {
                log().info("service process stopped");
                stopped = true;
                break;
            }
            sleep(1);
        }

        if (!stopped) {
            log().info("SIGTERM wasn't processed, use SIGKILL");
            kill(pid, SIGKILL);
            sleep(1);
        }
        _pid_file.remove();
    } else {
        log().error("couldn't stop process");
    }
}

};
