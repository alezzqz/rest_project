#include <syslog.h>
#include <sys/stat.h>
#include <iostream>

#include "restapi/router.h"
#include "restapi/server.h"
#include "service/service.h"
#include "logger/logger.h"

using namespace std;

logger::logger& log() {
    return logger::logger::get("cmserver");
}

void msg(const char* msg) {
    //cout << msg << endl;
    syslog(LOG_INFO, "%s", msg);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    //syslog_init sl;
    std::string cmd = argv[1];
    service::service srv("/var/run/cmservice.pid");

    if (cmd == "start") {
        auto res = srv.start();
        if (res == service::failed) {
            log().error("couldn't start service");
            return 1;
        }
        if (res == service::parent) {
            return 0;
        }

        restapi::api_server server("0.0.0.0", 8080, 4);
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
            log().info("REST API server starting...");
            server.run();
            log().info("REST API server stopped");
        });
        
        log().info("service started");
        srv.wait();
        log().info("stopping REST API server...");
        server.stop();
        api_thread.join();
        log().info("stopped");
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
        cout << "Unknown command. Use [start|stop]" << endl;
        return 2;
    }
    return 0;
}
