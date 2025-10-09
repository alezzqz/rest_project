#pragma once

#include "stop_signal.h"
#include "pid_file.h"

namespace service {

enum service_start_result {
    started = 0,
    failed = 1,
    parent = 2
};

class service {
    pid_file _pid_file;
    stop_signal _stop;

public:
    service(const std::string& pid_file_path);
    service_start_result start();
    void stop();
    void wait() {
        _stop.wait();
    }
};

};
