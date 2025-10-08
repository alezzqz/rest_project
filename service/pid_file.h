#pragma once

#include <string>

namespace service {

class pid_file {
    std::string _path;

public:
    pid_file(const std::string& path = "/var/run/cmservice.pid")
        : _path(path) {
    }

    void create(pid_t pid);
    void remove();
    pid_t get_pid();
};

};
