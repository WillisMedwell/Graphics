#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numbers>

namespace Cameras {
    

    class StationaryPerspective
    {
    private:
        constexpr static float Z_NEAR { 0.001f }, Z_FAR { 1000.0f };
        constexpr static glm::vec3 UP_DIRECTION { 0.0f, 1.0f, 0.0f };

    public:
        constexpr StationaryPerspective(glm::vec3 pos, glm::vec3 dir, float fov = 90.0f) noexcept
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

        inline auto increment_direction_via_angles(float delta_pitch, float delta_yaw) {
            constexpr float upper_bound = static_cast<float>(std::numbers::pi / 2.0f);
            constexpr float lower_bound = static_cast<float>(upper_bound * -1.0f);

            const float current_pitch = std::asin(direction.y);
            const float current_yaw = std::atan2(direction.z, direction.x);

            const float new_pitch = current_pitch + delta_pitch;
            const float new_yaw = std::clamp(current_yaw + delta_yaw, lower_bound, upper_bound);

            set_direction_via_angles(new_pitch, new_yaw);
        }
    };
}