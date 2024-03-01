#pragma once

#include <glm/mat4x4.hpp>

#include "Cameras/Fps.hpp"
#include "Cameras/Orbit.hpp"
#include "Cameras/StationaryPerspective.hpp"
#include "Cameras/Orthographic.hpp"
#include "Cameras/Isometric.hpp"

namespace Cameras {
    // Literally because i forget the order.
    auto get_mvp(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model) -> glm::mat4 {
        return projection * view * model;
    }
    auto get_mvp(const glm::mat4& projection, const glm::mat4& view) -> glm::mat4 {
        return projection * view;
    }

    template <typename T>
    concept IsCamera = requires(T t) {
        {
            t.projection_matrix(100, 100)
        } -> std::same_as<glm::mat4>;

        {
            t.view_matrix()
        } -> std::same_as<glm::mat4>;
    };

}

static_assert(Cameras::IsCamera<Cameras::StationaryPerspective>);
// static_assert(Cameras::IsCamera<Cameras::Fps>);
// static_assert(Cameras::IsCamera<Cameras::Orbit>);
