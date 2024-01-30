#pragma once

#include "Components/Components.hpp"
#include "Model/Static.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexBuffer.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <ranges>
#include <tuple>

namespace Renderer {
    class BatchDrawer
    {
    public:
        static auto to_3_digit(uint32_t num) noexcept -> std::array<char, 3> {
            assert(num < 1000);
            // via stack overflow: https://stackoverflow.com/a/1861020
            std::array<char, 3> result = { '0', '0', '0' };
            for (int i = result.size() - 1; i >= 0; i--) {
                result[i] = static_cast<char>(num % 10) + '0';
                num /= 10;
            }
            // could use std::from_chars but that requires a rotate so no thanks.
            return result;
        }
        using TexturedStaticModel = std::tuple<Model::Static&, Components::Transform&, Renderer::Texture&>;
#if 1
        template <size_t N>
        void batch(VertexBuffer& vb, IndexBuffer& ib, VertexArray& va, Shader& shader, std::array<TexturedStaticModel, N>& textured_models) {
            static std::vector<Model::BatchingVertex> vertices_buffer;
            static std::vector<Model::Index> indices_buffer;

            // Predetermine size for only one alloc.
            const size_t total_vertex_count = std::reduce(
                textured_models.begin(),
                textured_models.end(),
                size_t(0),
                [](const size_t& agg, const auto& tm) { return agg + std::get<0>(tm).vertices.size(); });
            const size_t total_index_count = std::reduce(
                textured_models.begin(),
                textured_models.end(),
                size_t(0),
                [](const size_t& agg, const auto& tm) { return agg + std::get<0>(tm).indices.size(); });
            vertices_buffer.resize(total_vertex_count);
            indices_buffer.resize(total_index_count);

            shader.bind();
            va.bind();
            vb.bind();
            ib.bind();

            // Move replacement info to compile time.
            constexpr static std::string_view mm_uniform_string = "u_model_transfrom[$$$]";
            constexpr static size_t mm_unifrom_string_size = mm_uniform_string.size();
            constexpr static std::ptrdiff_t mm_uniform_replacement_offset = std::distance(mm_uniform_string.begin(), std::ranges::find(mm_uniform_string, '$'));

            // Copy and ensure null ended string.
            std::array<char, mm_unifrom_string_size + 1> mm_uniform { '\0' };
            std::ranges::copy(mm_uniform_string, mm_uniform.begin());

            uint32_t i = 0;
            auto vert_iter = vertices_buffer.begin();
            auto indi_iter = indices_buffer.begin();
            for (auto& [model, transform, texture] : textured_models) {
                const auto tex_unit = texture.bind(true).on_error(Utily::ErrorHandler::print_then_quit).value();
                const auto index_offset = static_cast<Model::Index>(std::distance(vertices_buffer.begin(), vert_iter));

                // Pass model transform as uniform.
                auto i_chars = BatchDrawer::to_3_digit(i);
                std::ranges::copy(i_chars, mm_uniform.data() + mm_uniform_replacement_offset);
                shader.set_uniform(std::string_view { mm_uniform.data() }, transform.calc_transform_mat());

                // Account for index offset
                auto add_index_offset = [&](Model::Index index) { return index + index_offset; };
                indi_iter = std::ranges::copy(model.indices | add_index_offset, indi_iter);

                // Add texture index and model transform index
                auto add_tex_unit = [&](const Model::Vertex& v) { return Model::BatchingVertex { v.position, v.normal, v.uv_coord, tex_unit, i }; };
                vert_iter = std::ranges::copy(model.vertices | add_tex_unit, vert_iter);

                ++i;
            }

            // upload and draw.
            ib.load_indices(indices_buffer);
            vb.load_vertices(vertices_buffer);
            glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, (void*)0);

            // unlock the locked-bound textures.
            for (auto& [model, texture] : textured_models) {
                auto tex_unit = texture.bind(false).on_error(Utily::ErrorHandler::print_then_quit).value();
            }
        }
#endif
    };
}