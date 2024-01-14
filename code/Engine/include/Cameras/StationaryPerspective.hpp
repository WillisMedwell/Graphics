#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Cameras {
    class StationaryPerspective
    {
    private:
        glm::vec3 _position;
        glm::vec3 _direction;
        float _fov;

        constexpr static float Z_NEAR { 0.01f }, Z_FAR { 1000.0f };
        constexpr static glm::vec3 UP_DIRECTION { 0.0f, 1.0f, 0.0f };

    public:
        constexpr StationaryPerspective(glm::vec3 pos, glm::vec3 dir, float fov = 90.0f) noexcept
            : _position(pos)
            , _direction(dir)
            , _fov(fov) { }

        inline auto projection_matrix(auto width, auto height) const noexcept -> glm::mat4 {
            return glm::perspective(_fov, static_cast<float>(width) / static_cast<float>(height), Z_NEAR, Z_FAR);
        }
        inline auto view_matrix() const noexcept -> glm::mat4 {
            return glm::lookAt(_position, _direction, UP_DIRECTION);
        }
    };    
}