#pragma once

#include <string>
#include <vector>
#include <array>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace Models::Static {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv_coord;
    };

    static_assert(sizeof(Models::Static::Vertex) == sizeof(std::array<float,8>));

    struct Model {
        std::string name;
        std::vector<Vertex> vertex_data;
        std::vector<uint32_t> index_data;
    };

} // namespace Models::Static
