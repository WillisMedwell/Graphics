#pragma once

#include "Config.hpp"

#include <array>
#include <concepts>

namespace Renderer {

    template <typename T>
    concept isVec3f = requires(T t) {
        { t.x } -> std::same_as<float&>;
        { t.y } -> std::same_as<float&>;
        { t.z } -> std::same_as<float&>;
    };

    template <typename... Args>
        requires((std::same_as<float, Args> || std::same_as<uint32_t, Args> || isVec3f<Args>) && ...)
    class VertexBufferLayout
    {
        struct Element {
            uint32_t count;
            uint32_t type;
            uint32_t normalised;
        };

    private:
        using Types = std::tuple<Args...>;

        static_assert(std::tuple_size<Types>(), "Must provide some type arguments.");

        template <typename T>
        consteval static auto get_stride_of_type() -> uint32_t {
            if constexpr (std::same_as<T, float>) {
                return sizeof(float);
            } else if constexpr (std::same_as<T, uint32_t>) {
                return sizeof(uint32_t);
            } else if constexpr (isVec3f<T>) {
                return sizeof(float) * 3;
            }
            return 0;
        }

        template <typename T>
        consteval static auto get_element() -> Element {
            if constexpr (std::same_as<T, float>) {
                return Element { .count = 1, .type = GL_FLOAT, .normalised = GL_FALSE };
            } else if constexpr (std::same_as<T, uint32_t>) {
                return Element { .count = 1, .type = GL_UNSIGNED_INT, .normalised = GL_FALSE };
            } else if constexpr (isVec3f<T>) {
                return Element { .count = 3, .type = GL_FLOAT, .normalised = GL_FALSE };
            }
            return Element {};
        }

    public:
        consteval auto get_stride() -> uint32_t {
            return (get_stride_of_type<Args>() + ...);
        }

        consteval auto get_layout() {
            return std::to_array<Element>({ (get_element<Args>(), ...) });
        }
    };
}