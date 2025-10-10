#include <yaml-cpp/yaml.h>

#include "logger/logger.h"
#include "config.h"

extern logger::logger& log();

void config::read(const std::string& config_path) {
    log().debug("Reading config file {}...", config_path);

    auto config = YAML::LoadFile(config_path);
    api_http_address = config["api_http_address"].as<std::string>();
    api_http_port = config["api_http_port"].as<int>();
    api_threads = config["api_threads"].as<int>();
}
