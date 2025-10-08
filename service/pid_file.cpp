#include <syslog.h>
#include <sys/stat.h>

#include <fstream>
#include <filesystem>

#include "pid_file.h"

namespace service {

using namespace std;

void pid_file::create(pid_t pid) {
    ofstream file(_path);
    if (file.is_open()) {
        file << pid << std::endl;
        file.close();
        chmod(_path.c_str(), 0644);
    } else {
        syslog(LOG_ERR, "can't create pid file: %s", _path.c_str());
    }
}

void pid_file::remove() {
    if (filesystem::remove(_path.c_str()) == false) {
        syslog(LOG_ERR, "can't remove pid file: %s", _path.c_str());
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
