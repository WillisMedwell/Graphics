#pragma once

#include <array>
#include <glm/ext/quaternion_common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Models {
    using Vec2 = glm::vec2;
    using Vec3 = glm::vec3;
    using Quat = glm::quat;
    using Mat4 = glm::mat4x4;

    struct Vertex {
        Vec3 position;
        Vec3 normal;
        Vec2 uv_coord;
    };

    using Index = uint32_t;

    static_assert(sizeof(Vertex) == sizeof(std::array<float, 8>), "Unsupported alignment");
    static_assert(sizeof(Vertex) * 7 == sizeof(std::array<Vertex, 7>), "Unsupported packing");

}