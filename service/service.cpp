#include <syslog.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <filesystem>

#include "service.h"

namespace service {

using namespace std;

service::service(const std::string& pid_file_path)
    : _pid_file(pid_file_path) {
}

void service::start() {
    _pid_file.create(getpid());
}

void service::stop() {
    pid_t pid = _pid_file.get_pid();
    if (pid == 0) {
        cout << "not started" << endl;
        return;
    }

    if (kill(pid, SIGTERM) == 0) {
        cout << "stop signal sent" << endl;

        for (int i = 0; i < 10; ++i) {
            if (kill(pid, 0) != 0) {
                cout << "stopped" << endl;
                return;
            }
            sleep(1);
        }

        cout << "SIGTERM wasn't processed, use SIGKILL" << endl;
        kill(pid, SIGKILL);
        sleep(1);
        _pid_file.remove();
    } else {
        cout << "couldn't stop process" << endl;
    }
}

};
