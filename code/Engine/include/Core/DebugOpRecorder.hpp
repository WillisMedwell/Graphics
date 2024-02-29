#pragma once

#include "Config.hpp"
#include <string_view>
#include <tuple>
#include <vector>

namespace Core {
    class DebugOpRecorder
    {
    public:
        static inline auto instance() -> DebugOpRecorder& {
            static DebugOpRecorder dor {};
            return dor;
        }

        inline void clear() {
            if constexpr (Config::DEBUG_LEVEL == Config::DebugInfo::all) {
                _ops.clear();
            }
        }

        inline void push(std::string_view type [[maybe_unused]], std::string_view details [[maybe_unused]]) {
            if constexpr (Config::DEBUG_LEVEL == Config::DebugInfo::all) {
                try {
                    _ops.emplace_back(type, details);
                } catch (const std::exception& e) {
                    std::cerr << e.what() << '\n';
                }
            }
        }

        inline auto get_formatted_ops() {
            std::string res {};
            if constexpr (Config::DEBUG_LEVEL == Config::DebugInfo::all) {
                for (auto [core_type, details] : _ops) {
                    res += " - ";
                    res += core_type;
                    res += ": \t";
                    res += details;
                    res += "\n";
                }
            }
            return res;
        }

    private:
        using Ops = std::vector<std::tuple<std::string_view, std::string_view>>;

        Ops _ops;
        DebugOpRecorder() = default;
    };
}