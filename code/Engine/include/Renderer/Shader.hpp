#pragma once

#include <Utily/Utily.hpp>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace Renderer {
    class Shader
    {
        std::optional<int32_t> _program_id;
        std::unordered_map<size_t, int32_t> _cached_uniforms;

        enum class Type : uint32_t {
            frag = GL_FRAGMENT_SHADER,
            vert = GL_VERTEX_SHADER
        };

        static auto compile_shader(Type type, const std::string_view& source) -> Utily::Result<uint32_t, Utily::Error>;

    public:
        Shader() = default;
        Shader(const Shader&) = delete;
        Shader(Shader&&);

        auto init(const std::string_view& vert, const std::string_view& frag) -> Utily::Result<void, Utily::Error>;
        void stop();

        auto set_uniform(std::string_view uniform, int32_t value) -> Utily::Result<void, Utily::Error>;
        auto set_uniform(std::string_view uniform, float value) -> Utily::Result<void, Utily::Error>;
        auto set_uniform(std::string_view uniform, const glm::vec3& value) -> Utily::Result<void, Utily::Error>;
        auto set_uniform(std::string_view uniform, const glm::mat4& value) -> Utily::Result<void, Utily::Error>;

        ~Shader();
    };
}