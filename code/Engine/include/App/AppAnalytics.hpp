#pragma once

#include <string>

#include <sstream>

#include "Config.hpp"

class AppAnalytics
{
public:
    std::string gpu_info;

    auto get_gpu_info() {
        std::ostringstream info_stream;

        // Basic GPU Information
        const GLubyte* vendor = glGetString(GL_VENDOR);
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION);
        info_stream << "Vendor: " << vendor << "\n"
                    << "Renderer: " << renderer << "\n"
                    << "OpenGL Version: " << version << "\n";

        // Additional GPU Capabilities
        GLint value;

        // Max Texture Size
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
        info_stream << "Max Texture Size: " << value << "\n";

        // Max Number of Texture Image Units
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &value);
        info_stream << "Max Texture Image Units: " << value << "\n";

        // Max Cube Map Texture Size
        glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &value);
        info_stream << "Max Cube Map Texture Size: " << value << "\n";

        // Max Renderbuffer Size
        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &value);
        info_stream << "Max Renderbuffer Size: " << value << "\n";

        // Max Viewport Dimensions
        GLint maxViewportDims[2];
        glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);
        info_stream << "Max Viewport Dimensions: " << maxViewportDims[0] << " x " << maxViewportDims[1] << "\n";

        // Max Anisotropy
        GLfloat maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        info_stream << "Max Texture Anisotropy: " << maxAniso << "\n";

        // Max Samples in Multisample
        glGetIntegerv(GL_MAX_SAMPLES, &value);
        info_stream << "Max Samples in Multisample: " << value << "\n";

        // Max Color Attachments
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
        info_stream << "Max Color Attachments: " << value << "\n";

        // Max Draw Buffers
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &value);
        info_stream << "Max Draw Buffers: " << value << "\n";

        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
        info_stream << "Max Vertex Attributes: " << value << "\n";

        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &value);
        info_stream << "Max Vertex Uniform Components: " << value << "\n";

        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &value);
        info_stream << "Max Fragment Uniform Components: " << value << "\n";

        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &value);
        info_stream << "Max Vertex Texture Image Units: " << value << "\n";

        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &value);
        info_stream << "Max Combined Texture Image Units: " << value << "\n";

        // Fragment Processing
        glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &value);
        info_stream << "Max Fragment Input Components: " << value << "\n";

        glGetIntegerv(GL_MAX_TEXTURE_LOD_BIAS, &value);
        info_stream << "Max Texture LOD Bias: " << value << "\n";

        gpu_info = info_stream.str();
    }
};