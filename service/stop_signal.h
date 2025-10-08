#pragma once

#include <condition_variable>
#include <mutex>
#include <csignal>
#include <map>
#include <functional>

namespace service {

class stop_signal {
    bool _caught = false;
    std::condition_variable _cond;
    std::mutex _mut;
    static std::map<int, std::function<void(int)>> _handlers;

    static void handler_wrapper(int sig, siginfo_t* info, void* context);
    static bool handle_signal(int sig, std::function<void(int)> handler);
    static bool set_default_signal_handler(int sig);
    void signaled();
public:
    stop_signal();
    ~stop_signal();
    void wait();
    bool stopping() const {
        return _caught;
    }
};

};
