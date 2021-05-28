//
// Created by Evan Su on 26/5/21.
//

#include <iridium/logging.hpp>

std::shared_ptr<spdlog::logger> iridium::logger() {
  auto logger_name = "iridium";
  if (spdlog::get(logger_name) == nullptr) {
    spdlog::stdout_color_mt(logger_name);
  }
  return spdlog::get(logger_name);
}