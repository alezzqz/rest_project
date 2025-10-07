#include <syslog.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <condition_variable>
#include <mutex>
#include <csignal>
#include <filesystem>

#include "api_router.h"
#include "api_server.h"

using namespace std;

class stop_signal {
    bool _caught = false;
    std::condition_variable _cond;
    std::mutex _mut;
public:
    void signaled() {
        if (_caught) {
            return;
        }
        std::lock_guard<std::mutex> lock(_mut);
        _caught = true;
        _cond.notify_one();
    }
public:
    stop_signal() {
        // seastar::engine().handle_signal(SIGINT, [this] { signaled(); });
        // seastar::engine().handle_signal(SIGTERM, [this] { signaled(); });
    }
    ~stop_signal() {
        // There's no way to unregister a handler yet, so register a no-op handler instead.
        // seastar::engine().handle_signal(SIGINT, [] {});
        // seastar::engine().handle_signal(SIGTERM, [] {});
    }
    void wait() {
        std::unique_lock<std::mutex> lock(_mut);
        _cond.wait(lock, [this]{ return _caught; });
    }
    bool stopping() const {
        return _caught;
    }
};

class pid_file {
    std::string _path;

public:
    pid_file(const std::string& path = "/var/run/cmservice.pid")
        : _path(path) {
    }

    void create(pid_t pid) {
        ofstream file(_path);
        if (file.is_open()) {
            file << pid << std::endl;
            file.close();
            chmod(_path.c_str(), 0644);
        } else {
            syslog(LOG_ERR, "can't create pid file: %s", _path.c_str());
        }
    }

    void remove() {
        if (filesystem::remove(_path.c_str()) == false) {
            syslog(LOG_ERR, "can't remove pid file: %s", _path.c_str());
        }
    }

    pid_t get_pid() {
        ifstream file(_path);
        pid_t pid = 0;
        if (file.is_open()) {
            file >> pid;
            file.close();
        }
        return pid;
    }
};

class syslog_init {
public:
    syslog_init() {
        openlog("cmserver", LOG_PID | LOG_NDELAY, LOG_USER);
    }
    ~syslog_init() {
        closelog();
    }
};

stop_signal stop_sig;

void sighandler(int sig) {
    stop_sig.signaled();
}

void msg(const char* msg) {
    cout << msg << endl;
    //syslog(LOG_INFO, "%s", msg);
}

void stop(pid_t pid) {
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
    } else {
        cout << "couldn't stop process" << endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    syslog_init sl;
    std::string cmd = argv[1];
    pid_file pid_file;
    if (cmd == "start") {
        pid_file.create(getpid());

        signal(SIGINT, sighandler);
        signal(SIGTERM, sighandler);

        api_server server("0.0.0.0", 8080, 4);
        auto router = server.get_router();

        router->add_route(boost::beast::http::verb::get, "/api",
            [](const auto& req, const auto& match) {               
                auto resp = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
                resp->version(11);
                resp->result(boost::beast::http::status::ok);
                resp->set(boost::beast::http::field::server, "Boost REST Server");
                resp->set(boost::beast::http::field::content_type, "text/plain");
                resp->set(boost::beast::http::field::access_control_allow_origin, "*");
                resp->body() = "response";
                resp->prepare_payload();
                return resp;
            });

        thread api_thread([&server]{
            msg("api server thread started");
            server.run();
            msg("api server thread finished");
        });
        
        msg("test server started");
        stop_sig.wait();
        msg("stopping api server...");
        server.stop();
        api_thread.join();
        msg("test server finished");
    } else if (cmd == "stop") {
        pid_t pid = pid_file.get_pid();
        if (pid == 0) {
            msg("not started");
        } else {
            stop(pid);
            pid_file.remove();
        }
        return 0;
    } else if (cmd == "status") {
        pid_t pid = pid_file.get_pid();
        if (pid == 0) {
            msg("not started");
        } else if (kill(pid, 0) == 0){
            msg("started");
        } else {
            msg("not started but pid file exists");
        }
        return 0;
    } else {
        cout << "unknown command" << endl;
        return 2;
    }
    return 0;
}
