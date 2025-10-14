#include <iostream>
#include <yaml-cpp/yaml.h>
#include <boost/program_options.hpp>
#include <boost/range/algorithm.hpp>

#include "restapi/router.h"
#include "restapi/server.h"
#include "service/service.h"
#include "logger/logger.h"
#include "config.h"

using namespace std;
namespace po = boost::program_options;

logger::logger& log() {
    return logger::logger::get("cmagent");
}

void msg(const char* msg) {
    //cout << msg << endl;
    syslog(LOG_INFO, "%s", msg);
}

void setup_api_routes(std::shared_ptr<restapi::api_router> router) {
    router->add_route(boost::beast::http::verb::get, "/api/backup",
            [](const auto& req, const auto& match) {
                auto resp = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
                resp->version(11);
                resp->result(boost::beast::http::status::ok);
                resp->set(boost::beast::http::field::server, "Boost REST Agent");
                resp->set(boost::beast::http::field::content_type, "text/plain");
                resp->set(boost::beast::http::field::access_control_allow_origin, "*");
                resp->body() = "backup agent response";
                resp->prepare_payload();
                return resp;
            });
}

int on_start(const po::variables_map& vm, service::service& srv) {
    config cfg;
    try {
        cfg.read(vm["config-file"].as<std::string>());
    } catch (const YAML::Exception& e) {
        cout << "can't read config file: " << e.what() << endl;
        return 1;
    }

    auto res = srv.start();
    if (res == service::failed) {
        log().error("couldn't start service");
        return 1;
    }
    if (res == service::parent) {
        return 0;
    }

    restapi::api_server server(cfg.api_http_address, cfg.api_http_port, cfg.api_threads);
    auto router = server.get_router();
    setup_api_routes(router);

    thread api_thread([&server] {
        server.run();
    });

    log().info("service started");
    srv.wait();
    log().info("stopping REST API server...");
    server.stop();
    api_thread.join();
    log().info("stopped");
    return 0;
}

int main(int argc, char** argv) {
    po::options_description cmdline_visible("Command line options");
    cmdline_visible.add_options()
        ("help", "produce help message")
        ("config-file", po::value<std::string>()->default_value("cmagent.yaml"), "config file");

    po::options_description cmdline_hidden("Hidden options");
    cmdline_hidden.add_options()
        ("action", "action");

    po::options_description cmdline_options("All options");
    cmdline_options.add(cmdline_visible).add(cmdline_hidden);

    po::positional_options_description p;
    p.add("action", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
        po::notify(vm);
    } catch (po::error& e) {
        cout << e.what() << endl;
        return 1;
    }

    if (vm.count("help") || !vm.count("action")) {
        cout << "Usage: cmagent <action> [OPTIONS]" << endl;
        cout << "Actions: start, stop" << endl;
        cout << cmdline_visible << endl;
        return 0;
    }

    auto action = vm["action"].as<std::string>();
    std::vector<std::string> actions{"start", "stop"};
    if (boost::find(actions, action) == actions.end()) {
        cout << "Unknown action. Use [start|stop]" << endl;
        return 1;
    }

    service::service srv("/var/run/cmagent.pid");

    if (action == "start") {
        return on_start(vm, srv);
    } else if (action == "stop") {
        srv.stop();
    }
    return 0;
}
