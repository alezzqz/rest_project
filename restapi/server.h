#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace restapi {

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
    
    std::shared_ptr<api_router> get_router() { return _router; }

private:
    void start_accept();
    void accept_handler(beast::error_code ec, tcp::socket socket);
    
    net::io_context _ioc;
    tcp::acceptor _acceptor;
    std::vector<std::thread> _threads;
    std::shared_ptr<api_router> _router;
    std::string _address;
    unsigned short _port;
    int _thread_count;
    bool _stopped;
};

};
