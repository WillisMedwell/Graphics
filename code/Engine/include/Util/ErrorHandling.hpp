#pragma once

#include <concepts>
#include <format>
#include <iostream>
#include <source_location>

#include "Config.hpp"

namespace Util {

    struct ErrorMsg {
        std::string msg;

        ErrorMsg(std::string&& error, [[maybe_unused]] std::source_location location = std::source_location::current()) {
            if constexpr (Config::DEBUG_LEVEL == Config::DebugInfo::NONE) {
                msg = std::format("Error: {}.", error);
            } else {
                msg = std::format(
                    "Error: {}. Created at \"{}\", function \"{}\", on line {}",
                    error,
                    location.file_name(),
                    location.function_name(),
                    location.line());
            }
        }
    };

    namespace ErrorHandling {

        auto print = [](auto error) {
            if constexpr (std::is_same_v<ErrorMsg, std::decay_t<decltype(error)>>) {
                std::cerr << error.msg << std::endl;
            } else {
                std::cerr << error << std::endl;
            }
        };
    }
}