#include "Renderer/Shader.hpp"

#include "Config.hpp"

using namespace std::literals;


namespace Renderer {
    Shader::Shader(Shader&& other)
        : _program_id(std::exchange(other._program_id, std::nullopt))
        , _cached_uniforms(std::move(other._cached_uniforms)) { }

    auto Shader::compile_shader(Type type, const std::string_view& source) -> Utily::Result<uint32_t, Utily::Error> {
        constexpr static auto shader_verison =
#if defined(CONFIG_TARGET_NATIVE)
            "#version 300 es \n"sv;
#elif defined(CONFIG_TARGET_WEB)
            "#version 300 core \n"sv;
#else
            "#bad version\n"sv;
#endif
        static std::string versioned_source { shader_verison };
        versioned_source.resize(shader_verison.size());
        versioned_source.append(source);

        uint32_t shader = glCreateShader((int32_t)type);
        const char* src = versioned_source.data();

        glShaderSource(shader, versioned_source.size(), &src, nullptr);
        glCompileShader(shader);

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::NONE) {
            int32_t compile_status = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

            if (compile_status == GL_FALSE) {
                int32_t length = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
                std::vector<char> error_chars(length);
                glGetShaderInfoLog(shader, length, &length, error_chars.data());

                std::cerr
                    << "Shader failed to compile: "
                    << "\""
                    << path.string()
                    << "\".\n"
                    << std::string_view(error_chars)
                    << '\n'
                    << std::endl;

                glDeleteShader(shader);
                exit(EXIT_FAILURE);
            }
        }
        return shader;
    }

    auto Shader::init(const std::string_view& vert, const std::string_view& frag) -> Utily::Result<void, Utily::Error> {
        if (_program_id) {
            return Utily::Error { "Trying to override in-use shader" };
        }
    }

    void Shader::stop() {
    }

    auto Shader::set_uniform(std::string_view uniform, int32_t value) -> Utily::Result<void, Utily::Error> {
    }
    auto Shader::set_uniform(std::string_view uniform, float value) -> Utily::Result<void, Utily::Error> {
    }
    auto Shader::set_uniform(std::string_view uniform, const glm::vec3& value) -> Utily::Result<void, Utily::Error> {
    }
    auto Shader::set_uniform(std::string_view uniform, const glm::mat4& value) -> Utily::Result<void, Utily::Error> {
    }

    ~Shader::Shader() {
    }

} // namespace Renderer
