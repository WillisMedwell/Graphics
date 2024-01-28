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

#include <Utily/Utily.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Model {
    auto decode_as_static_model(std::span<uint8_t> file_data, Utily::StaticVector<char, 16> file_extension)
        -> Utily::Result<Static, Utily::Error> {
        constexpr auto assimp_process_flags =
            aiProcess_CalcTangentSpace
            | aiProcess_Triangulate
            | aiProcess_JoinIdenticalVertices // maybe bad.
            | aiProcess_SortByPType
            | aiProcess_OptimizeGraph
            | aiProcess_OptimizeMeshes
            | aiProcess_GenNormals
            | aiProcess_GenBoundingBoxes;

        Assimp::Importer importer {};

        file_extension.emplace_back('\0'); // make null terminated.

        const aiScene* assimp_scene = importer.ReadFileFromMemory(
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
        auto assimp_meshes = std::span { assimp_scene->mMeshes, assimp_scene->mNumMeshes };

        Static loaded_model;

        if (assimp_meshes.size() > 1) {
            return Utily::Error { "There was multiple model mesh data. Use function decode_as_static_models." };
        } else if (assimp_meshes.size() == 0) {
            return Utily::Error { "There was no model mesh data" };
        }

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
        return loaded_model;
    }
}

// using namespace std::literals;

// auto isModelFormatSupported(const std::string_view& extension) -> bool {
//     static constexpr auto supported_model_extensions = std::to_array({ ".fbx"sv }); //,".gltf"sv });

//     auto is_same_extension = [&](const auto& ext) {
//         return extension == ext;
//     };

//     return std::ranges::any_of(supported_model_extensions, is_same_extension);
// }

// auto readRawFile(const std::filesystem::path& path) -> Utily::Result<std::vector<char>, Utily::Error> {
//     if (!std::filesystem::exists(path)) {
//         return Utily::Error("The file does not exist");
//     }

//     std::ifstream infile(path, std::ios::binary);

//     if (!infile.is_open()) {
//         return Utily::Error("The file is in use");
//     }

//     std::vector<char> raw_contents;

//     infile.seekg(0, std::ios::end);
//     raw_contents.resize(infile.tellg());
//     infile.seekg(0, std::ios::beg);
//     infile.read(raw_contents.data(), raw_contents.size());

//     if (!infile.good()) {
//         return Utily::Error("Error reading the file");
//     }

//     return raw_contents;
// }

// auto importAnimationData(Models::Staging::Model* model_ptr, std::span<aiAnimation*> assimp_animations) noexcept {
//     Models::Staging::Model& model = *model_ptr;

//     for (const auto& assimp_animation : assimp_animations) {

//         const auto assimp_channels = std::span { assimp_animation->mChannels, assimp_animation->mNumChannels };
//         const auto assimp_mesh_channels = std::span { assimp_animation->mMeshChannels, assimp_animation->mNumMeshChannels };
//         const auto assimp_mesh_morph_channels = std::span { assimp_animation->mMorphMeshChannels, assimp_animation->mNumMorphMeshChannels };
//     }
// }

// auto importMeshData(Models::Staging::Model* model_ptr, std::span<aiMesh*> assimp_meshes) noexcept {
//     Models::Staging::Model& model = *model_ptr;

//     for (const auto& assimp_mesh : assimp_meshes) {
//         const auto assimp_vertices = std::span { assimp_mesh->mVertices, assimp_mesh->mNumVertices };
//         const auto assimp_uv_coords = std::span { assimp_mesh->mTextureCoords[0], assimp_mesh->mNumVertices };
//         const auto assimp_normals = std::span { assimp_mesh->mNormals, assimp_mesh->mNumVertices };

//         for (const auto& [v, n, uv] : std::views::zip(assimp_vertices, assimp_normals, assimp_uv_coords)) {
//             model.vertex_data.emplace_back(
//                 Models::Staging::Vertex {
//                     .position = { v.x, v.y, v.z },
//                     .normal = { n.x, n.y, n.z },
//                     .uv_coord = { uv.x, uv.y } });
//         }

//         const auto assimp_faces = std::span { assimp_mesh->mFaces, assimp_mesh->mNumFaces };

//         for (const auto& assimp_face : assimp_faces) {
//             for (const auto& index : std::span { assimp_face.mIndices, assimp_face.mNumIndices }) {
//                 model.index_data.emplace_back(index);
//             }
//         }
//     }
// }

// namespace Models::Staging {
//     auto loadModel(const std::filesystem::path& path) -> Utily::Result<Staging::Model, Utily::Error> {
//         const auto ext = path.extension().string();

//         if (!isModelFormatSupported(ext)) {
//             return Utily::Error {
//                 std::format(
//                     "Not going to load model \"{}\", the format is not supported.",
//                     path.string())
//             };
//         }

//         auto maybe_file_contents = readRawFile(path);

//         if (maybe_file_contents.has_error()) {
//             return Utily::Error {
//                 std::format(
//                     "Failed to load model \"{}\". The error was \"{}\".",
//                     path.string(),
//                     maybe_file_contents.error().what())
//             };
//         }
//         const auto& file_contents = maybe_file_contents.value();

//         Assimp::Importer importer {};

//         const aiScene* assimp_scene = importer.ReadFileFromMemory(
//             file_contents.data(),
//             file_contents.size(),
//             aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType,
//             ext.c_str());

//         if (assimp_scene == nullptr) {
//             return Utily::Error {
//                 std::format(
//                     "Failed to load model \"{}\". The error was: \"{}\"",
//                     path.string(),
//                     importer.GetErrorString())
//             };
//         }

//         Staging::Model model;

//         auto assimp_meshes = std::span { assimp_scene->mMeshes, assimp_scene->mNumMeshes };
//         auto assimp_animations = std::span { assimp_scene->mAnimations, assimp_scene->mNumAnimations };

//         importMeshData(&model, assimp_meshes);
//         importAnimationData(&model, assimp_animations);

//         model.has_animations = false;

//         return model;
//     }
// }
