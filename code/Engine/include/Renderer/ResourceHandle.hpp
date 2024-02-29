#pragma once

#include "Core/Core.hpp"
#include <optional>

namespace Renderer {
    template <typename T>
    struct ResourceHandle {
        std::optional<int> id = std::nullopt;
        size_t resource_owner_id = 0;
    };
}