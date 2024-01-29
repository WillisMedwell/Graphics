#include "Renderer/FrameBuffer.hpp"

#include <array>
#include <utility>

namespace Renderer {
    constexpr static uint32_t INVALID_BUFFER_ID = 0;

    static FrameBuffer* last_bound = nullptr;

    struct ColourAttachment {
        std::optional<uint32_t> id = std::nullopt;
        bool in_use = false;
    };

    static Utily::StaticVector<ColourAttachment, 64> colour_attachments;

    auto get_usable_colour_attachment() -> Utily::Result<std::tuple<ColourAttachment*, uint32_t>, Utily::Error> {
        if (!colour_attachments.size()) {
            int32_t max_colour_attachments = 0;
            glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_colour_attachments);
            colour_attachments.resize(max_colour_attachments);
            colour_attachments[0].in_use = true;
            return std::tuple(&colour_attachments[0], 0);
        }
        auto is_usable = [](auto& ca) {
            return ca.in_use == false;
        };
        auto iter = std::ranges::find_if(colour_attachments, is_usable);

        if (iter == colour_attachments.end()) [[unlikely]] {
            return Utily::Error { "Ran out of usable colour attachments." };
        }
        iter->in_use = true;
        return std::tuple {
            &colour_attachments[0],
            std::distance(colour_attachments.begin(), iter)
        };
    }

    FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
        : _id(std::exchange(other._id, std::nullopt))
        , _colour_attachment_index(std::exchange(other._id, std::nullopt)) {
        last_bound = nullptr;
    }

    auto FrameBuffer::init(uint32_t width, uint32_t height) noexcept -> Utily::Result<void, Utily::Error> {
        if (_id) {
            return Utily::Error { "Trying to override in-use frame buffer" };
        }

        _id = INVALID_BUFFER_ID;
        glGenFramebuffers(1, &_id.value());
        if (_id.value() == INVALID_BUFFER_ID) {
            _id = std::nullopt;
            return Utily::Error { "Failed to create Frame buffer. glGenFramebuffers failed." };
        }

        if (!_colour_attachment_index) {
            auto result = get_usable_colour_attachment();
            if (result.has_error()) {
                return result.error();
            }
            _colour_attachment_index = std::get<uint32_t>(result.value());
        }

        auto& [colour_rb, in_use] = colour_attachments[*_colour_attachment_index];

        bind();
        if (!colour_rb) {
            colour_rb = 0;
            glGenRenderbuffers(1, &colour_rb.value());
        }
        glBindRenderbuffer(GL_RENDERBUFFER, *colour_rb);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_SRGB8_ALPHA8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + *_colour_attachment_index, GL_RENDERBUFFER, *colour_rb);
        in_use = true;
        _width = width;
        _height = height;
        return {};
    }

    void FrameBuffer::stop() noexcept {
        if (_id.value_or(INVALID_BUFFER_ID) != INVALID_BUFFER_ID) {
            glDeleteFramebuffers(1, &_id.value());
        }
        _id = std::nullopt;

        if (_colour_attachment_index
            && _colour_attachment_index.value() < colour_attachments.size()) {
            colour_attachments[*_colour_attachment_index].in_use = false;
        }

        if (last_bound == this) {
            last_bound = this;
        }
    }

    FrameBuffer::~FrameBuffer() noexcept {
        stop();
    }

    void FrameBuffer::bind() noexcept {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_BUFFER_ID) == INVALID_BUFFER_ID) {
                std::cerr << "Trying to bind invalid frame buffer.";
                assert(false);
            }
        }
        if (last_bound != this) {
            glBindFramebuffer(GL_FRAMEBUFFER, _id.value_or(INVALID_BUFFER_ID));
            last_bound = this;
        }
    }

    void FrameBuffer::unbind() noexcept {
        if constexpr (Config::SKIP_UNBINDING) {
            return;
        } else if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_BUFFER_ID) == INVALID_BUFFER_ID) {
                std::cerr << "Trying to unbind invalid frame buffer.";
                assert(false);
            }
        }

        if (last_bound != nullptr) {
            glBindBuffer(GL_FRAMEBUFFER, 0);
            last_bound = nullptr;
        }
    }

    uint32_t ScreenFrameBuffer::width = 0;
    uint32_t ScreenFrameBuffer::height = 0;

    void ScreenFrameBuffer::clear(glm::vec4 colour) noexcept {
        ScreenFrameBuffer::bind();
        glClearColor(colour.x, colour.y, colour.z, colour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void ScreenFrameBuffer::bind() noexcept {
        if (last_bound != nullptr) {
            glBindBuffer(GL_FRAMEBUFFER, 0);
            last_bound = nullptr;
        }
    }
    void ScreenFrameBuffer::resize(uint32_t screen_width, uint32_t screen_height) noexcept {
        if (screen_width != ScreenFrameBuffer::width || screen_height != ScreenFrameBuffer::height) {
            ScreenFrameBuffer::bind();
            glViewport(0, 0, screen_width, screen_height);
            ScreenFrameBuffer::width = screen_width;
            ScreenFrameBuffer::height = screen_height;
        }
    }

}