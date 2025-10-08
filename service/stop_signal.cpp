#include "stop_signal.h"

namespace service {

std::map<int, std::function<void(int)>> stop_signal::_handlers;

stop_signal::stop_signal() {
    handle_signal(SIGINT, [this](int) { signaled(); });
    handle_signal(SIGTERM, [this](int) { signaled(); });
}

stop_signal::~stop_signal() {
    set_default_signal_handler(SIGINT);
    set_default_signal_handler(SIGTERM);
}

void stop_signal::wait() {
    std::unique_lock<std::mutex> lock(_mut);
    _cond.wait(lock, [this]{ return _caught; });
}

bool stop_signal::handle_signal(int sig, std::function<void(int)> handler) {
    _handlers[sig] = handler;

    struct sigaction sa;
    sa.sa_sigaction = handler_wrapper;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    return sigaction(sig, &sa, nullptr) == 0;
}

bool stop_signal::set_default_signal_handler(int sig) {
    struct sigaction default_action;
    default_action.sa_handler = SIG_DFL;
    sigemptyset(&default_action.sa_mask);
    default_action.sa_flags = 0;

    if (sigaction(sig, &default_action, nullptr) == 0) {
        _handlers.erase(sig);
        return true;
    }
    return false;
}

void stop_signal::handler_wrapper(int sig, siginfo_t* info, void* context) {
    auto it = _handlers.find(sig);
    if (it != _handlers.end() && it->second) {
        it->second(sig);
    }
}

void stop_signal::signaled() {
    if (_caught) {
        return;
    }
    std::lock_guard<std::mutex> lock(_mut);
    _caught = true;
    _cond.notify_one();
}

};
