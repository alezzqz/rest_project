#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class api_router;

class api_server {
public:
    api_server(const std::string& address, unsigned short port, int threads = 1);
    ~api_server();
    
    void run();
    void stop();
    
    std::shared_ptr<api_router> get_router() { return router_; }

private:
    void start_accept();
    void accept_handler(beast::error_code ec, tcp::socket socket);
    
    net::io_context ioc_;
    tcp::acceptor acceptor_;
    std::vector<std::thread> threads_;
    std::shared_ptr<api_router> router_;
    std::string address_;
    unsigned short port_;
    int thread_count_;
    bool stopped_;
};
