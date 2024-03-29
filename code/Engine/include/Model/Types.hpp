#pragma once

#include "Core/VertexBufferLayout.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>

#include <iostream>
#include <array>

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
        using VBL = Core::VertexBufferLayout<glm::vec3, glm::vec3, glm::vec2>;
    };

    struct Vertex2D {
        Vec2 position;
        Vec2 uv_coord;

        friend auto operator<<(std::ostream& stream, const Vertex2D& vertex) -> std::ostream& {
            stream << "v(" << vertex.position.x << ',' << vertex.position.y << ") \t"
                   << "uv(" << vertex.uv_coord.x << ',' << vertex.uv_coord.y << ")";
            return stream;
        }
        using VBL = Core::VertexBufferLayout<glm::vec2, glm::vec2>;
    };

    struct BatchingVertex {
        Vec3 position;
        Vec3 normal;
        Vec2 uv_coord;
        uint32_t texture_unit_index;
        uint32_t model_transform_index;

        friend auto operator<<(std::ostream& stream, const BatchingVertex& vertex) -> std::ostream& {
            stream << "v(" << vertex.position.x << ',' << vertex.position.y << ',' << vertex.position.z << ") \t"
                   << "n(" << vertex.normal.x << ',' << vertex.normal.y << ',' << vertex.normal.z << ") \t"
                   << "uv(" << vertex.uv_coord.x << ',' << vertex.uv_coord.y << ") \t"
                   << "tui(" << vertex.texture_unit_index << ") \t"
                   << "mti(" << vertex.model_transform_index << ")";

            return stream;
        }

        using VBL = Core::VertexBufferLayout<glm::vec3, glm::vec3, glm::vec2, uint32_t, uint32_t>;
    };

    using Index = uint32_t;

    static_assert(sizeof(Vertex) == sizeof(std::array<float, 8>), "Unsupported alignment");
    static_assert(sizeof(Vertex) * 7 == sizeof(std::array<Vertex, 7>), "Unsupported packing");

}