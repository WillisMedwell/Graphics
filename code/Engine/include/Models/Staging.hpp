#pragma once

#include "Util/Util.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Models::Staging {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv_coord;
    };

    struct Action {
        std::string name;
    };

    struct Model {
        std::string name;
        std::vector<Vertex> vertex_data;
        std::vector<uint32_t> index_data;

        bool has_animations = false;
    };

    [[nodiscard]] auto loadModel(const std::filesystem::path& path) -> Util::Result<Staging::Model, std::string>;
} // Models::Staging
