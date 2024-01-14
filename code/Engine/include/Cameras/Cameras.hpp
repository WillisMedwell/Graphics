#pragma once


#include <glm/mat4x4.hpp>

#include "Cameras/Fps.hpp"
#include "Cameras/Orbit.hpp"
#include "Cameras/StationaryPerspective.hpp"

namespace Cameras{

    template<typename T>
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

