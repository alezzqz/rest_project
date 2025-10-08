#include "server.h"
#include "router.h"
#include <iostream>
#include <memory>

class http_session : public std::enable_shared_from_this<http_session> {
private:
    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    std::shared_ptr<api_router> router_;

public:
    http_session(tcp::socket socket, std::shared_ptr<api_router> router)
        : socket_(std::move(socket)), router_(router) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        req_ = {};
        buffer_.consume(buffer_.size());

        http::async_read(socket_, buffer_, req_,
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
            std::cerr << "Read error: " << ec.message() << std::endl;
            return;
        }

        request_handler();
    }

    void request_handler() {
        try {
            auto response = router_->request_handler(std::move(req_));
            do_write(response);
        } catch (const std::exception& e) {
            std::cerr << "Request handling error: " << e.what() << std::endl;
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

        http::async_write(socket_, *response,
            [self, response](beast::error_code ec, std::size_t bytes_transferred) {
                self->on_write(ec, bytes_transferred, response->need_eof());
            });
    }

    void on_write(beast::error_code ec, std::size_t, bool close) {
        if (ec) {
            std::cerr << "Write error: " << ec.message() << std::endl;
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
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        // ignore errors on close
    }
};

api_server::api_server(const std::string& address, unsigned short port, int threads)
    : ioc_(threads)
    , acceptor_(ioc_)
    , router_(std::make_shared<api_router>())
    , address_(address)
    , port_(port)
    , thread_count_(threads)
    , stopped_(false) {
    
    tcp::endpoint endpoint(net::ip::make_address(address), port);
    beast::error_code ec;
    
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        throw std::runtime_error("Failed to open acceptor: " + ec.message());
    }
    
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        throw std::runtime_error("Failed to set socket option: " + ec.message());
    }
    
    acceptor_.bind(endpoint, ec);
    if (ec) {
        throw std::runtime_error("Failed to bind acceptor: " + ec.message());
    }
    
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        throw std::runtime_error("Failed to listen: " + ec.message());
    }
    
    std::cout << "Server initialized on " << address << ":" << port << std::endl;
}

api_server::~api_server() {
    stop();
}

void api_server::run() {
    start_accept();
    
    threads_.reserve(thread_count_);
    for (int i = 0; i < thread_count_; ++i) {
        threads_.emplace_back([this]() {
            ioc_.run();
        });
    }
    
    std::cout << "Server started with " << thread_count_ << " threads" << std::endl;
    
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void api_server::stop() {
    if (!stopped_) {
        stopped_ = true;
        ioc_.stop();
    }
}

void api_server::start_accept() {
    if (stopped_) return;
    
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            accept_handler(ec, std::move(socket));
        });
}

void api_server::accept_handler(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        if (ec != net::error::operation_aborted) {
            std::cerr << "Accept error: " << ec.message() << std::endl;
        }
    } else {
        std::make_shared<http_session>(std::move(socket), router_)->start();
    }
    
    if (!stopped_) {
        start_accept();
    }
}
