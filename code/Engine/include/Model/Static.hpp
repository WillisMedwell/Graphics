#pragma once

#include "Model/Types.hpp"
#include "Utily/Utily.hpp"

namespace Model {
    // Contiguous vertices and indices.
    struct Static {
        std::array<Vec3, 2> axis_align_bounding_box;
        std::unique_ptr<std::byte[]> data;
        std::span<Vertex> vertices;
        std::span<uint32_t> indices;
    };

    auto decode_as_static_model(
        std::span<uint8_t> file_data,
         Utily::StaticVector<char, 16> file_extension)
        -> Utily::Result<Static, Utily::Error>;

}