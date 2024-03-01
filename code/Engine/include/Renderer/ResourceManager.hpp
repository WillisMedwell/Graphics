#pragma once

#include "Renderer/ResourceHandle.hpp"

#include <Utily/Utily.hpp>
#include <functional>
#include <random>
#include <string>
#include <string>

namespace Renderer {

    template <typename T, typename... Args>
    concept CanBeInitWithArgs = requires(T t, Args&&... args) {
        t.init(std::forward<Args>(args)...);
    };

    struct Panic {
        void operator()(Utily::Error& error) {
            std::cerr << error.what() << std::endl;
            throw std::runtime_error(std::string{error.what()});
        }
        void operator()(auto& error) {
            throw std::runtime_error("Render panic");
        }
    };

    class ResourceManager
    {
    private:
        constexpr static size_t MAX_S = 32;
        constexpr static size_t MAX_T = 32;
        constexpr static size_t MAX_VB = 32;
        constexpr static size_t MAX_VA = 32;
        constexpr static size_t MAX_IB = 32;

#if 0
        Utily::StaticVector<Core::Shader, MAX_S> _shaders;
        Utily::StaticVector<Core::Texture, MAX_T> _textures;
        Utily::StaticVector<Core::VertexArray, MAX_VA> _vertex_arrays;
        Utily::StaticVector<Core::IndexBuffer, MAX_IB> _index_buffers;
        Utily::StaticVector<Core::VertexBuffer, MAX_VB> _vertex_buffers;
#else
        std::vector<Core::Shader> _shaders;
        std::vector<Core::Texture> _textures;
        std::vector<Core::VertexArray> _vertex_arrays;
        std::vector<Core::IndexBuffer> _index_buffers;
        std::vector<Core::VertexBuffer> _vertex_buffers;
#endif

        size_t owner_id;

        template <typename T>
        inline constexpr auto& get_resource_buffer() {
            if constexpr (std::same_as<T, Core::Shader>) {
                return _shaders;
            } else if constexpr (std::same_as<T, Core::Texture>) {
                return _textures;
            } else if constexpr (std::same_as<T, Core::VertexArray>) {
                return _vertex_arrays;
            } else if constexpr (std::same_as<T, Core::VertexBuffer>) {
                return _vertex_buffers;
            } else if constexpr (std::same_as<T, Core::IndexBuffer>) {
                return _index_buffers;
            } else {
                throw std::runtime_error("That resource is not mapped.");
            }
        }

    public:
        template <typename T, typename... Args>
        [[nodiscard]] inline auto create_and_init_resource(Args&&... args) -> std::tuple<ResourceHandle<T>, T&> {
            auto& resource_buffer = get_resource_buffer<T>();
            ResourceHandle<T> handle { resource_buffer.size(), this->owner_id };

            resource_buffer.emplace_back();

            T& resource = resource_buffer[handle.id.value()];

            static_assert(CanBeInitWithArgs<T, Args...>, "The Args must be params for the method T::init()");
            resource.init(std::forward<Args>(args)...)
                .on_error([](auto& error) {
                    throw std::runtime_error(std::string(error.what()));
                });

            return std::tuple<ResourceHandle<T>, T&>(handle, resource);
        }

        template <typename T>
        [[nodiscard]] inline auto get_resource(ResourceHandle<T> handle) -> T& {
            auto& resource_buffer = get_resource_buffer<T>();
            assert(handle.resource_owner_id == this->owner_id);
            if (handle.id) {
                return resource_buffer[handle.id.value()];
            } else {
                throw std::runtime_error("Invalid resource handle");
            }
        }

        template <typename... T>
        [[nodiscard]] inline auto get_resources(ResourceHandle<T>... handles) -> std::tuple<T&...> {
            return std::tuple<T&...>(get_resource(handles)...);
        }

        template <typename T>
        inline void free_resource(ResourceHandle<T> handle) {
            auto& resource_buffer = get_resource_buffer<T>();
            assert(handle.resource_owner_id == this->owner_id);
            if (handle.id) {
                resource_buffer[handle.id.value()].stop();
            }
        }

        template <typename... T>
        inline void free_resources(ResourceHandle<T>... handles) {
            (..., free_resource(handles));
        }

        ResourceManager() {
            static std::mt19937 rng(std::random_device {}());
            static std::uniform_int_distribution<size_t> dist { 0, std::numeric_limits<size_t>::max() };
            owner_id = dist(rng);
        }
    };
}