#include "Staging.hpp"

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
    static constexpr auto supported_model_extensions = std::to_array({ ".fbx"sv, ".gltf"sv });

    auto is_same_extension = [&](const auto& ext) {
        return extension == ext;
    };

    return std::ranges::any_of(supported_model_extensions, is_same_extension);
}

auto readRawFile(const std::filesystem::path& path) -> Result<std::vector<char>, std::string_view> {
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

namespace Staging {

    auto loadModel(const std::filesystem::path& path) -> Result<Staging::Model, std::string> {
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
        auto& file_contents = maybe_file_contents.value();

        Assimp::Importer importer {};

        importer.ReadFileFromMemory(
            file_contents.data(),
            file_contents.size(),
            aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType,
            ext.c_str());

        const aiScene* assimp_scene = importer.GetScene();

        if (assimp_scene == nullptr) {
            return std::format(
                "Failed to load model \"{}\". The error was: \"{}\"",
                path.string(),
                importer.GetErrorString());
        }

        const auto assimp_meshes = std::span { assimp_scene->mMeshes, assimp_scene->mNumMeshes };
        const auto assimp_animations = std::span { assimp_scene->mAnimations, assimp_scene->mNumAnimations };

        return "TODO"s;
    }
}
