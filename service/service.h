#pragma once

#include "stop_signal.h"
#include "pid_file.h"

namespace service {

class service {
    pid_file _pid_file;
    stop_signal _stop;

public:
    service(const std::string& pid_file_path);
    void start();
    void stop();
    void wait() {
        _stop.wait();
    }
};

};
