#include <syslog.h>
#include <sys/stat.h>

#include <fstream>
#include <filesystem>

#include "pid_file.h"

#include "logger/logger.h"

namespace service {

using namespace std;

logger::logger& log() {
    return logger::logger::get("cmserver");
}

void pid_file::create(pid_t pid) {
    log().debug("creating pid file {}", _path);

    ofstream file(_path);
    if (file.is_open()) {
        file << pid << std::endl;
        file.close();
        chmod(_path.c_str(), 0644);
    } else {
        log().error("can't create pid file: {}", _path);
    }
}

void pid_file::remove() {
    log().debug("removing pid file {}", _path);

    if (filesystem::remove(_path.c_str()) == false) {
        log().error("can't remove pid file: {}", _path);
    }
}

pid_t pid_file::get_pid() {
    ifstream file(_path);
    pid_t pid = 0;
    if (file.is_open()) {
        file >> pid;
        file.close();
    }
    return pid;
}

};
