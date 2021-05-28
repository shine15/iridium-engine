//
// Created by Evan Su on 26/5/21.
//

#ifndef IRIDIUM_INCLUDE_IRIDIUM_LOGGING_HPP_
#define IRIDIUM_INCLUDE_IRIDIUM_LOGGING_HPP_

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace iridium {
std::shared_ptr<spdlog::logger> logger();
}  // namespace iridium

#endif //IRIDIUM_INCLUDE_IRIDIUM_LOGGING_HPP_
