#include <syslog.h>
#include <sys/stat.h>
#include <iostream>

#include "restapi/router.h"
#include "restapi/server.h"
#include "service/service.h"

using namespace std;

class syslog_init {
public:
    syslog_init() {
        openlog("cmserver", LOG_PID | LOG_NDELAY, LOG_USER);
    }
    ~syslog_init() {
        closelog();
    }
};

void msg(const char* msg) {
    cout << msg << endl;
    //syslog(LOG_INFO, "%s", msg);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    syslog_init sl;
    std::string cmd = argv[1];
    service::service srv("/var/run/cmservice.pid");

    if (cmd == "start") {
        srv.start();
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
        //stop_sig.wait();
        srv.wait();
        msg("stopping api server...");
        server.stop();
        api_thread.join();
        msg("test server finished");
    } else if (cmd == "stop") {
        srv.stop();
        return 0;
    } else if (cmd == "status") {
        // pid_t pid = pid_file.get_pid();
        // if (pid == 0) {
        //     msg("not started");
        // } else if (kill(pid, 0) == 0){
        //     msg("started");
        // } else {
        //     msg("not started but pid file exists");
        // }
        return 0;
    } else {
        cout << "unknown command" << endl;
        return 2;
    }
    return 0;
}
