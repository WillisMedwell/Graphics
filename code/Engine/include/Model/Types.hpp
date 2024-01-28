#pragma once

#include <array>
#include <iostream>
#include <glm/ext/quaternion_common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/VertexBufferLayout.hpp"

namespace Model {
    using Vec2 = glm::vec2;
    using Vec3 = glm::vec3;
    using Quat = glm::quat;
    using Mat4 = glm::mat4x4;

    struct Vertex {
        Vec3 position;
        Vec3 normal;
        Vec2 uv_coord;

        friend auto operator<<(std::ostream& stream, const Vertex& vertex) -> std::ostream& {
            stream << "v(" << vertex.position.x << ',' << vertex.position.y << ',' << vertex.position.z << ") \t"
                   << "n(" << vertex.normal.x << ',' << vertex.normal.y << ',' << vertex.normal.z << ") \t"
                   << "uv(" << vertex.uv_coord.x << ',' << vertex.uv_coord.y << ")";

            return stream;
        }
    };

    using StaticVertexLayout = Renderer::VertexBufferLayout<glm::vec3, glm::vec3, glm::vec2>;

    using Index = uint32_t;

    static_assert(sizeof(Vertex) == sizeof(std::array<float, 8>), "Unsupported alignment");
    static_assert(sizeof(Vertex) * 7 == sizeof(std::array<Vertex, 7>), "Unsupported packing");

}