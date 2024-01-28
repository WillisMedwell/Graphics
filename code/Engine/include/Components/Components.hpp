#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Components {
    struct Transform {
        glm::vec3 position = { 0, 0, 0 };
        glm::vec3 scale = { 1, 1, 1 };
        glm::quat rotation = { 0, 0, 0, 1 };

        auto calc_transform_mat() -> glm::mat4 {
            glm::mat4 pos = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 sca = glm::scale(glm::mat4(1.0f), scale);
            glm::mat4 rot = glm::toMat4(rotation);
            return pos * rot * sca;
        }
    };

    struct Spinning {
        glm::vec3 axis_of_rotation;
        double angle;
        double rotations_per_second;

        auto update(double dt) -> Spinning& {
            angle += rotations_per_second * 360.0 * dt;
            return *this;
        }

        auto calc_quat() -> glm::quat {
            return glm::angleAxis(static_cast<float>(glm::radians(angle)), axis_of_rotation);
        }
    };

    struct PointLightShadow {
        glm::vec3 colour;
        float intensity;
    };

    struct PointLightAmbient {
        glm::vec3 colour;
        float intensity;
    };
};