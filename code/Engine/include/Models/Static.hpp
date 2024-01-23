#pragma once

#include "Models/Types.hpp"
#include "Utily/Utily.hpp"

namespace Models {

    struct Static {
        std::array<Vec3, 2> axis_align_bounding_box;
        std::unique_ptr<std::byte[]> data;
        std::span<Vertex> vertices;
        std::span<uint32_t> indices;
    };

    auto decode_as_static_model(
        std::span<std::byte> file_data,
         Utily::StaticVector<char, 16> file_extension)
        -> Utily::Result<Static, Utily::Error>;
}