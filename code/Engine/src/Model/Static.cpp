#include "Model/Static.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <ranges>
#include <span>

#include "Profiler/Profiler.hpp"
#include <Utily/Utily.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Model {
    auto decode_as_static_model(std::span<uint8_t> file_data, Utily::StaticVector<char, 16> file_extension)
        -> Utily::Result<Static, Utily::Error> {

        Profiler::Timer timer("Model::decode_as_static_model()", { "rendering" });

        constexpr auto assimp_process_flags =
            aiProcess_CalcTangentSpace
            | aiProcess_Triangulate
            | aiProcess_JoinIdenticalVertices
            | aiProcess_SortByPType
            | aiProcess_OptimizeGraph
            | aiProcess_OptimizeMeshes
            | aiProcess_GenNormals
            | aiProcess_GenBoundingBoxes;


        
        Assimp::Importer importer {};

        file_extension.emplace_back('\0'); // make null terminated.

        const aiScene* assimp_scene = nullptr;

        {
            Profiler::Timer assimp_timer("Assimp::Importer::ReadFileFromMemory()");

            assimp_scene = importer.ReadFileFromMemory(
                file_data.data(),
                file_data.size(),
                assimp_process_flags,
                &(*file_extension.begin()));

            if (assimp_scene == nullptr) {
                return Utily::Error {
                    std::format(
                        "Failed to load model. The error was: \"{}\"",
                        importer.GetErrorString())
                };
            }
        }

        auto assimp_meshes = std::span { assimp_scene->mMeshes, assimp_scene->mNumMeshes };

        Static loaded_model;

        if (assimp_meshes.size() > 1) {
            return Utily::Error { "There was multiple model mesh data. Use function decode_as_static_models." };
        } else if (assimp_meshes.size() == 0) {
            return Utily::Error { "There was no model mesh data" };
        }

        {
            Profiler::Timer extract_timer("extract_from_assimp_meshes()", { "rendering" });
            for (const auto& assimp_mesh : assimp_meshes) {

                if (!assimp_mesh->HasFaces() || !assimp_mesh->HasNormals() || !assimp_mesh->HasPositions()) {
                    return Utily::Error("There was either no: faces || normals || positions");
                }

                auto faces = std::span { assimp_mesh->mFaces, assimp_mesh->mNumFaces };
                auto positions = std::span { assimp_mesh->mVertices, assimp_mesh->mNumVertices };
                auto normals = std::span { assimp_mesh->mNormals, assimp_mesh->mNumVertices };
                auto uvs = std::span { assimp_mesh->mTextureCoords[0], assimp_mesh->mNumVertices };

                auto get_formatted_aabb = [&] {
                    return std::array<Vec3, 2> {
                        Vec3 { static_cast<float>(assimp_mesh->mAABB.mMin.x), static_cast<float>(assimp_mesh->mAABB.mMin.y), static_cast<float>(assimp_mesh->mAABB.mMin.z) },
                        Vec3 { static_cast<float>(assimp_mesh->mAABB.mMax.x), static_cast<float>(assimp_mesh->mAABB.mMax.y), static_cast<float>(assimp_mesh->mAABB.mMax.z) },
                    };
                };
                auto to_span = [](const aiFace& face) -> std::span<uint32_t> {
                    return { face.mIndices, face.mNumIndices };
                };

                auto to_vert = [](auto&& pnu) -> Vertex {
                    auto& [p, n, u] = pnu;
                    return {
                        .position = { p.x, p.y, p.z },
                        .normal = { n.x, n.y, n.z },
                        .uv_coord = { u.x, u.y }
                    };
                };
                auto vertices_view = std::views::zip(positions, normals, uvs) | std::views::transform(to_vert);

                auto indices_view = faces | std::views::transform(to_span) | std::views::join;
                auto [ptr, vert, ind] = Utily::InlineArrays::alloc_copy(std::move(vertices_view), std::move(indices_view));

                loaded_model.axis_align_bounding_box = get_formatted_aabb();
                loaded_model.data = std::move(ptr);
                loaded_model.vertices = vert;
                loaded_model.indices = ind;
            }
        }
        return loaded_model;
    }

    auto generate_plane(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) -> Model::Static {
        glm::vec3 normal = glm::normalize(glm::vec3(-1) * glm::cross(b - a, c - a));

        std::array<Model::Vertex, 4> vertices = {
            Model::Vertex { .position = a, .normal = normal, .uv_coord = glm::vec2(0.0f, 0.0f) },
            Model::Vertex { .position = b, .normal = normal, .uv_coord = glm::vec2(1.0f, 0.0f) },
            Model::Vertex { .position = c, .normal = normal, .uv_coord = glm::vec2(1.0f, 1.0f) },
            Model::Vertex { .position = d, .normal = normal, .uv_coord = glm::vec2(0.0f, 1.0f) }
        };
        std::array<Model::Index, 6> indices = { 0, 1, 2, 2, 3, 0 };

        auto [data, verts, indis] = Utily::InlineArrays::alloc_copy(vertices, indices);
        return Model::Static {
            .axis_align_bounding_box = {},
            .data = std::move(data),
            .vertices = verts,
            .indices = indis
        };
    }

    auto join(Static&& lhs, Static&& rhs) -> Model::Static {

        const uint32_t index_offset = lhs.vertices.size();

        auto [d, v, i] = Utily::InlineArrays::alloc_uninit<Model::Vertex, Model::Index>(
            lhs.vertices.size() + rhs.vertices.size(),
            lhs.indices.size() + rhs.indices.size());

        { // copy the vertices.
            auto iter = std::uninitialized_copy(lhs.vertices.begin(), lhs.vertices.end(), v.begin());
            std::uninitialized_copy(rhs.vertices.begin(), rhs.vertices.end(), iter);
        }
        { // copy indices, adding the offset to the rhs
            auto iter = std::uninitialized_copy(lhs.indices.begin(), lhs.indices.end(), i.begin());

            for (const auto& i : rhs.indices) {
                std::construct_at(&(*iter), i + index_offset);
                ++iter;
            }
        }

        lhs.data.reset();
        lhs.vertices = {};
        lhs.indices = {};

        rhs.data.reset();
        rhs.vertices = {};
        rhs.indices = {};

        return Model::Static {
            .axis_align_bounding_box = {}, // TODO
            .data = std::move(d),
            .vertices = v,
            .indices = i
        };
    }
}
