#include "Renderer/Shader.hpp"

#include "Config.hpp"

#include <format>

using namespace std::literals;

namespace Renderer {

    static Shader* last_bound_s = nullptr;

    Shader::Shader(Shader&& other)
        : _program_id(std::exchange(other._program_id, std::nullopt))
        , _cached_uniforms(std::move(other._cached_uniforms)) {
        last_bound_s = nullptr;
    }

    auto Shader::compile_shader(Type type, const std::string_view& source) -> Utily::Result<uint32_t, Utily::Error> {
        constexpr static auto shader_verison =
#if defined(CONFIG_TARGET_NATIVE)
            "#version 330 core \n"sv;
#elif defined(CONFIG_TARGET_WEB)
            "#version 300 es \n"sv;
#else
            "#bad version \n"sv;
#endif
        static std::string versioned_source { shader_verison };
        versioned_source.resize(shader_verison.size());
        versioned_source.append(source);

        const char* src = versioned_source.data();

        uint32_t shader = glCreateShader((int32_t)type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            int32_t compile_status = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

            if (compile_status == GL_FALSE) {
                int32_t length = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
                std::string error_msg;
                error_msg.resize(length);
                glGetShaderInfoLog(shader, length, &length, error_msg.data());
                glDeleteShader(shader);
                return Utily::Error {
                    std::format(
                        "Shader failed to compile where the error was:\n \"{}\"",
                        error_msg)
                };
            }
        }
        return shader;
    }

    auto Shader::init(const std::string_view& vert, const std::string_view& frag) -> Utily::Result<void, Utily::Error> {
        if (_program_id) {
            return Utily::Error { "Trying to override in-use shader" };
        }

        _program_id = glCreateProgram();

        Utily::Result vr = Shader::compile_shader(Shader::Type::vert, vert);
        Utily::Result fr = Shader::compile_shader(Shader::Type::frag, frag);

        if (vr.has_error()) {
            stop();
            return vr.error();
        } else if (fr.has_error()) {
            stop();
            return vr.error();
        }

        glAttachShader(_program_id.value(), vr.value());
        glAttachShader(_program_id.value(), fr.value());
        glLinkProgram(_program_id.value());
        glValidateProgram(_program_id.value());

        glDeleteShader(vr.value());
        glDeleteShader(fr.value());

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            int32_t is_valid = GL_FALSE;
            glGetProgramiv(_program_id.value(), GL_VALIDATE_STATUS, &is_valid);
            Utily::Error error {};
            if (!is_valid) {
                GLint msg_length = 0;
                glGetProgramiv(_program_id.value(), GL_INFO_LOG_LENGTH, &msg_length);

                std::string error_msg;
                error_msg.resize(msg_length);
                glGetProgramInfoLog(_program_id.value(), msg_length, nullptr, error_msg.data());

                Utily::Error error {
                    std::format(
                        "Unable to validate the program when constructing the shader. Where the error was: \n{}",
                        error_msg)
                };
                stop();
                return error;
            }
        }

        return {};
    }

    // cache it so we dont query the GPU over already bound stuff.

    void Shader::bind() noexcept {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (!_program_id.has_value()) {
                std::cerr << "Trying to use invalid program";
                assert(_program_id.has_value());
            }
        }
        if (last_bound_s != this) {
            last_bound_s = this;
        }
        glUseProgram(_program_id.value());
    }
    void Shader::unbind() noexcept {
        if constexpr (Config::SKIP_UNBINDING) {
            return;
        } else if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (!_program_id.has_value()) {
                std::cerr << "Trying to use invalid program";
                assert(_program_id.has_value());
            }
        }

        if (last_bound_s != this && last_bound_s != nullptr) {
            glUseProgram(0);
            last_bound_s = nullptr;
        }
    }
    void Shader::stop() {
        if (_program_id) {
            _cached_uniforms.clear();
            glDeleteProgram(*_program_id);
            _program_id = std::nullopt;
        }
        if (last_bound_s == this) {
            last_bound_s = nullptr;
        }
    }

    // Assumes shader is bound already.
    auto Shader::get_uniform(const std::string_view uniform) noexcept -> Utily::Result<Uniform, Utily::Error> {
        size_t uniform_hash = std::hash<std::string_view> {}(uniform);
        Uniform& ul = _cached_uniforms[uniform_hash];

        if (ul.location == -1) {
            ul.location = glGetUniformLocation(_program_id.value(), uniform.data());
            if (ul.location == -1) {
                return Utily::Error { std::format("Invalid Uniform key: {}", uniform) };
            }
        }
        return ul;
    }

    auto Shader::set_uniform(std::string_view uniform, int32_t value) noexcept -> Utily::Result<void, Utily::Error> {
        bind();
        auto maybe_uniform = get_uniform(uniform);
        if (maybe_uniform.has_error()) {
            return maybe_uniform.error();
        }
        glUniform1i(maybe_uniform.value().location, value);
        return {};
    }
    auto Shader::set_uniform(std::string_view uniform, float value) noexcept -> Utily::Result<void, Utily::Error> {
        bind();
        auto maybe_uniform = get_uniform(uniform);
        if (maybe_uniform.has_error()) {
            return maybe_uniform.error();
        }
        glUniform1f(maybe_uniform.value().location, value);
        return {};
    }
    auto Shader::set_uniform(std::string_view uniform, const glm::vec3& value) noexcept -> Utily::Result<void, Utily::Error> {
        bind();
        auto maybe_uniform = get_uniform(uniform);
        if (maybe_uniform.has_error()) {
            return maybe_uniform.error();
        }
        glUniform3f(maybe_uniform.value().location, value.x, value.y, value.z);
        return {};
    }
    auto Shader::set_uniform(std::string_view uniform, const glm::mat4& value) noexcept -> Utily::Result<void, Utily::Error> {
        bind();
        auto maybe_uniform = get_uniform(uniform);
        if (maybe_uniform.has_error()) {
            return maybe_uniform.error();
        }
        glUniformMatrix4fv(maybe_uniform.value().location, 1, GL_FALSE, (const float*)&value[0][0]);
        return {};
    }

    Shader::~Shader() {
        stop();
    }

} // namespace Renderer
