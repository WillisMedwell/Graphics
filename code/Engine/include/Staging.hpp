#pragma once

#include "Result.hpp"

#include <string>
#include <filesystem>
#include <vector>
#include <cstdint>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace Staging {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv_coord;
    };
    
    struct Model {
        std::string name;
        std::vector<Vertex> vertex_data;
        std::vector<uint32_t> index_data;
    };

    [[nodiscard]] auto loadModel(const std::filesystem::path& path) -> Result<Staging::Model, std::string>;
}