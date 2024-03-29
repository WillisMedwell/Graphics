#pragma once

#include "Core/Core.hpp"

namespace Renderer {

    template <typename T>
    struct ResourceHandle;
    
    class ResourceManager;
    class FontRenderer;
}

#include "Renderer/ResourceHandle.hpp"
#include "Renderer/ResourceManager.hpp"
#include "Renderer/FontBatchRenderer.hpp"
#include "Renderer/InstanceRenderer.hpp"
