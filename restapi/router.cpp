#include "router.h"
#include <iostream>

#include "logger/logger.h"

namespace restapi {

logger::logger& log() {
    return logger::logger::get("cmserver");
}

api_router::api_router() {
    // add basic routes here
}

void api_router::add_route(http::verb method, const std::string& pattern, Handler handler) {
    Route route{std::regex(pattern), handler};
    routes_[method].push_back(route);
}

std::shared_ptr<http::response<http::string_body>> 
api_router::request_handler(http::request<http::string_body>&& req) {
    log().debug("Request: {} {}", boost::beast::http::to_string(req.method()).to_string(), req.target().to_string());
    
    auto method_it = routes_.find(req.method());
    if (method_it == routes_.end()) {
        return method_not_allowed_response(req);
    }
    
    const auto target = std::string(req.target().data(), req.target().size());
    for (const auto& route : method_it->second) {
        std::smatch match;
        if (std::regex_match(target, match, route.pattern)) {
            return route.handler(req, match);
        }
    }
    
    return not_found_response(req);
}

std::shared_ptr<http::response<http::string_body>> 
api_router::create_response(http::status status, const std::string& body, 
                       const std::string& content_type) {
    auto response = std::make_shared<http::response<http::string_body>>();
    response->version(11);
    response->result(status);
    response->set(http::field::server, "Test REST API Server");
    response->set(http::field::content_type, content_type);
    response->set(http::field::access_control_allow_origin, "*");
    response->body() = body;
    response->prepare_payload();
    return response;
}

std::shared_ptr<http::response<http::string_body>> 
api_router::not_found_response(const http::request<http::string_body>& req) {
    return create_response(http::status::not_found, "The requested resource was not found", "text/plain");
}

std::shared_ptr<http::response<http::string_body>> 
api_router::method_not_allowed_response(const http::request<http::string_body>& req) {
    return create_response(http::status::method_not_allowed, "The requested method is not supported for this resource", "text/plain");
}

};
