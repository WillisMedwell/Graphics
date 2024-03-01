#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numbers>

namespace Cameras {
    class Isometric
    {
    private:
        constexpr static float Z_NEAR { 0.001f }, Z_FAR { 1000.0f };
        constexpr static glm::vec3 UP_DIRECTION { 0.0f, 1.0f, 0.0f };

    public:
        constexpr Isometric(glm::vec3 pos, glm::vec3 dir, float fov = 90.0f) noexcept
            : position(pos)
            , direction(dir + glm::vec3 { 0.000f, 0.001f, -0.001f })
            , fov(fov) { }

        inline auto projection_matrix(auto width, auto height) const noexcept -> glm::mat4 {
            return glm::perspective(fov, static_cast<float>(width) / static_cast<float>(height), Z_NEAR, Z_FAR);
        }
        inline auto view_matrix() const noexcept -> glm::mat4 {
            
            return glm::lookAt(position, position + direction, UP_DIRECTION);
        }

        glm::vec3 direction;
        glm::vec3 position;
        float fov;

        inline auto set_direction_via_angles(float pitch, float yaw) {
            direction.x = cos(yaw) * cos(pitch);
            direction.y = sin(pitch);
            direction.z = sin(yaw) * cos(pitch);

            direction = glm::normalize(direction);
        }
    };
}