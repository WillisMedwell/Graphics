#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>

#include <Utily/Utily.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Config.hpp"

namespace Core {
    class Shader
    {
    public:
        Shader() = default;
        Shader(const Shader&) = delete;
        Shader(Shader&&);

        [[nodiscard]] auto init(const std::string_view& vert, const std::string_view& frag) -> Utily::Result<void, Utily::Error>;
        void stop();
        void bind() noexcept;
        void unbind() noexcept;
        auto set_uniform(std::string_view uniform, int32_t value) noexcept -> Utily::Result<void, Utily::Error>;
        auto set_uniform(std::string_view uniform, float value) noexcept -> Utily::Result<void, Utily::Error>;
        auto set_uniform(std::string_view uniform, const glm::vec3& value) noexcept -> Utily::Result<void, Utily::Error>;
        auto set_uniform(std::string_view uniform, const glm::vec4& value) noexcept -> Utily::Result<void, Utily::Error>;
        auto set_uniform(std::string_view uniform, const glm::mat4& value) noexcept -> Utily::Result<void, Utily::Error>;


        ~Shader();

    private:
        struct Uniform {
            int32_t location = -1;
        };
        enum class Type : uint32_t {
            frag = GL_FRAGMENT_SHADER,
            vert = GL_VERTEX_SHADER
        };
        std::optional<int32_t> _program_id = std::nullopt;
        std::unordered_map<size_t, Uniform> _cached_uniforms;

        [[nodiscard]] static auto compile_shader(Type type, const std::string_view& source) -> Utily::Result<uint32_t, Utily::Error>;
        [[nodiscard]] auto get_uniform(const std::string_view uniform) noexcept -> Utily::Result<Uniform, Utily::Error>;
    };
}