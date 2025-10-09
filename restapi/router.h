#pragma once

#include <boost/beast/http.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <regex>

namespace restapi {

namespace beast = boost::beast;
namespace http = beast::http;

class api_router {
public:
    using Handler = std::function<std::shared_ptr<http::response<http::string_body>>(
        const http::request<http::string_body>& req,
        const std::smatch& match)>;

    api_router();
    
    void add_route(http::verb method, const std::string& pattern, Handler handler);
    std::shared_ptr<http::response<http::string_body>> 
    request_handler(http::request<http::string_body>&& req);

private:
    struct Route {
        std::regex pattern;
        Handler handler;
    };
    
    std::unordered_map<http::verb, std::vector<Route>> routes_;
    
    std::shared_ptr<http::response<http::string_body>> 
    create_response(http::status status, const std::string& body, 
                   const std::string& content_type = "application/json");
    
    std::shared_ptr<http::response<http::string_body>> 
    not_found_response(const http::request<http::string_body>& req);
    
    std::shared_ptr<http::response<http::string_body>> 
    method_not_allowed_response(const http::request<http::string_body>& req);
};

};
