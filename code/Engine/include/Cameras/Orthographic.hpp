#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numbers>

namespace Cameras {
    class Orthographic
    {
        constexpr static float Z_NEAR { 0.001f }, Z_FAR { 10.0f };

    public:
        inline auto projection_matrix(float width, float height) noexcept -> glm::mat4 {
            return glm::ortho(0.0f, width, 0.0f, height, Z_NEAR, Z_FAR);
        }

        Orthographic() = default;
    };
}