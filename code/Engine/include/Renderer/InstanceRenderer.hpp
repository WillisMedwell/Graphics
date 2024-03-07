#pragma once

#include "Core/Core.hpp"
#include "Media/Media.hpp"
#include "Renderer/ResourceManager.hpp"

namespace Renderer {
    class InstanceRenderer
    {
    public:
        void init(ResourceManager& resource_manager, const Model::Static& model, Media::Image& image);
        void stop(ResourceManager& resource_manager);

        void push_instance(const glm::mat4& instance_transformation);
        void draw_instances(ResourceManager& resource_manager, glm::vec2 screen_dimensions);
    private:
        Renderer::ResourceHandle<Core::Shader> _s;
        Renderer::ResourceHandle<Core::Texture> _t;
        Renderer::ResourceHandle<Core::VertexBuffer> _vb_mesh; // the mesh of the instance.
        Renderer::ResourceHandle<Core::VertexBuffer> _vb_transforms; // the matrix transformation of each instance.
        Renderer::ResourceHandle<Core::IndexBuffer> _ib;
        Renderer::ResourceHandle<Core::VertexArray> _va;

        std::vector<glm::mat4> _current_instances;
    };
}