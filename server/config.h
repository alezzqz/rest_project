#pragma once

#include <string>

struct config {
    std::string api_http_address;
    int api_http_port;
    int api_threads;

    void read(const std::string& config_path);
};
