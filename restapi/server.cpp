#include "server.h"
#include "router.h"
#include <iostream>
#include <memory>

#include "logger/logger.h"

namespace restapi {

extern logger::logger& log();

class http_session : public std::enable_shared_from_this<http_session> {
private:
    tcp::socket _socket;
    beast::flat_buffer _buffer;
    http::request<http::string_body> _req;
    std::shared_ptr<api_router> _router;

public:
    http_session(tcp::socket socket, std::shared_ptr<api_router> router)
        : _socket(std::move(socket)), _router(router) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        _req = {};
        _buffer.consume(_buffer.size());

        http::async_read(_socket, _buffer, _req,
            beast::bind_front_handler(
                &http_session::on_read,
                shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        if (ec == http::error::end_of_stream) {
            do_close();
            return;
        }

        if (ec) {
            log().error("Http session read error: {}", ec.message());
            return;
        }

        request_handler();
    }

    void request_handler() {
        try {
            auto response = _router->request_handler(std::move(_req));
            do_write(response);
        } catch (const std::exception& e) {
            log().error("Request handling error: {}", e.what());
            auto response = std::make_shared<http::response<http::string_body>>();
            response->version(11);
            response->result(http::status::internal_server_error);
            response->set(http::field::server, "Boost REST Server");
            response->set(http::field::content_type, "application/json");
            response->body() = R"({"error":"Internal Server Error"})";
            response->prepare_payload();
            do_write(response);
        }
    }

    void do_write(std::shared_ptr<http::response<http::string_body>> response) {
        auto self = shared_from_this();

        http::async_write(_socket, *response,
            [self, response](beast::error_code ec, std::size_t bytes_transferred) {
                self->on_write(ec, bytes_transferred, response->need_eof());
            });
    }

    void on_write(beast::error_code ec, std::size_t, bool close) {
        if (ec) {
            log().error("Http session write error: {}", ec.message());
            return;
        }

        if (close) {
            do_close();
            return;
        }

        do_read();
    }

    void do_close() {
        beast::error_code ec;
        _socket.shutdown(tcp::socket::shutdown_send, ec);
        // ignore errors on close
    }
};

api_server::api_server(const std::string& address, unsigned short port, int threads)
    : _ioc(threads)
    , _acceptor(_ioc)
    , _router(std::make_shared<api_router>())
    , _address(address)
    , _port(port)
    , _thread_count(threads)
    , _stopped(false) {
    
    tcp::endpoint endpoint(net::ip::make_address(address), port);
    beast::error_code ec;
    
    _acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        throw std::runtime_error("Failed to open acceptor: " + ec.message());
    }
    
    _acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        throw std::runtime_error("Failed to set socket option: " + ec.message());
    }
    
    _acceptor.bind(endpoint, ec);
    if (ec) {
        throw std::runtime_error("Failed to bind acceptor: " + ec.message());
    }
    
    _acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        throw std::runtime_error("Failed to listen: " + ec.message());
    }
    
    log().info("REST API server initialized on {}:{}", address, port);
}

api_server::~api_server() {
    stop();
}

void api_server::run() {
    log().info("starting REST API server...");
    start_accept();
    
    _threads.reserve(_thread_count);
    for (int i = 0; i < _thread_count; ++i) {
        _threads.emplace_back([this]() {
            _ioc.run();
        });
    }
    
    log().info("REST API server started with {} threads", _thread_count);
    
    for (auto& thread : _threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    log().info("REST API server stopped");
}

void api_server::stop() {
    if (!_stopped) {
        _stopped = true;
        _ioc.stop();
    }
}

void api_server::start_accept() {
    if (_stopped) return;
    
    _acceptor.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            accept_handler(ec, std::move(socket));
        });
}

void api_server::accept_handler(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        if (ec != net::error::operation_aborted) {
            log().error("REST API Accept error: {}", ec.message());
        }
    } else {
        std::make_shared<http_session>(std::move(socket), _router)->start();
    }
    
    if (!_stopped) {
        start_accept();
    }
}

};
