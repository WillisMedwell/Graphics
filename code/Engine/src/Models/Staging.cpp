#include "Models/Staging.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <array>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <ranges>
#include <span>

using namespace std::literals;

auto isModelFormatSupported(const std::string_view& extension) -> bool {
    static constexpr auto supported_model_extensions = std::to_array({ ".fbx"sv }); //,".gltf"sv });

    auto is_same_extension = [&](const auto& ext) {
        return extension == ext;
    };

    return std::ranges::any_of(supported_model_extensions, is_same_extension);
}

auto readRawFile(const std::filesystem::path& path) -> Util::Result<std::vector<char>, std::string_view> {
    if (!std::filesystem::exists(path)) {
        return "The file does not exist"sv;
    }

    std::ifstream infile(path, std::ios::binary);

    if (!infile.is_open()) {
        return "The file is in use"sv;
    }

    std::vector<char> raw_contents;

    infile.seekg(0, std::ios::end);
    raw_contents.resize(infile.tellg());
    infile.seekg(0, std::ios::beg);
    infile.read(raw_contents.data(), raw_contents.size());

    if (!infile.good()) {
        return "Error reading the file"sv;
    }

    return raw_contents;
}

auto importAnimationData(Models::Staging::Model* model_ptr, std::span<aiAnimation*> assimp_animations) noexcept {
    Models::Staging::Model& model = *model_ptr;

    for (const auto& assimp_animation : assimp_animations) {

        const auto assimp_channels = std::span { assimp_animation->mChannels, assimp_animation->mNumChannels };
        const auto assimp_mesh_channels = std::span { assimp_animation->mMeshChannels, assimp_animation->mNumMeshChannels };
        const auto assimp_mesh_morph_channels = std::span { assimp_animation->mMorphMeshChannels, assimp_animation->mNumMorphMeshChannels };
    }
}

auto importMeshData(Models::Staging::Model* model_ptr, std::span<aiMesh*> assimp_meshes) noexcept {
    Models::Staging::Model& model = *model_ptr;

    for (const auto& assimp_mesh : assimp_meshes) {
        const auto assimp_vertices = std::span { assimp_mesh->mVertices, assimp_mesh->mNumVertices };
        const auto assimp_uv_coords = std::span { assimp_mesh->mTextureCoords[0], assimp_mesh->mNumVertices };
        const auto assimp_normals = std::span { assimp_mesh->mNormals, assimp_mesh->mNumVertices };

        for (const auto& [v, n, uv] : std::views::zip(assimp_vertices, assimp_normals, assimp_uv_coords)) {
            model.vertex_data.emplace_back(
                Models::Staging::Vertex {
                    .position = { v.x, v.y, v.z },
                    .normal = { n.x, n.y, n.z },
                    .uv_coord = { uv.x, uv.y } });
        }

        const auto assimp_faces = std::span { assimp_mesh->mFaces, assimp_mesh->mNumFaces };

        for (const auto& assimp_face : assimp_faces) {
            for (const auto& index : std::span { assimp_face.mIndices, assimp_face.mNumIndices }) {
                model.index_data.emplace_back(index);
            }
        }
    }
}

namespace Models::Staging {
    auto loadModel(const std::filesystem::path& path) -> Util::Result<Staging::Model, std::string> {
        const auto ext = path.extension().string();

        if (!isModelFormatSupported(ext)) {
            return std::format(
                "Not going to load model \"{}\", the format is not supported.",
                path.string());
        }

        auto maybe_file_contents = readRawFile(path);

        if (maybe_file_contents.has_error()) {
            return std::format(
                "Failed to load model \"{}\". The error was \"{}\".",
                path.string(),
                maybe_file_contents.error());
        }
        const auto& file_contents = maybe_file_contents.value();

        Assimp::Importer importer {};

        const aiScene* assimp_scene = importer.ReadFileFromMemory(
            file_contents.data(),
            file_contents.size(),
            aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType,
            ext.c_str());

        if (assimp_scene == nullptr) {
            return std::format(
                "Failed to load model \"{}\". The error was: \"{}\"",
                path.string(),
                importer.GetErrorString());
        }

        Staging::Model model;

        auto assimp_meshes = std::span { assimp_scene->mMeshes, assimp_scene->mNumMeshes };
        auto assimp_animations = std::span { assimp_scene->mAnimations, assimp_scene->mNumAnimations };

        importMeshData(&model, assimp_meshes);

        importAnimationData(&model, assimp_animations);

        model.has_animations = false;

        return model;
    }
}
