#include <gtest/gtest.h>

#include <App.hpp>

using namespace std::literals;

struct OneSecondAppData {
    std::chrono::steady_clock::time_point start_time;
};
struct OneSecondAppLogic {
    void init(AppRenderer& renderer, OneSecondAppData& data) {
        constexpr auto vert =
            "precision mediump float; "
            "layout(location = 0) in vec3 aPos;"
            "void main() {"
            "    gl_Position = vec4(aPos, 1.0);"
            "}"sv;

        constexpr auto frag =
            "precision mediump float; "
            "out vec4 FragColor;  "
            " void main()"
            " {"
            "     FragColor = vec4(1.0, 0.5, 0.2, 1.0); "
            " }"sv;

        EXPECT_FALSE(renderer.add_shader(vert, frag).has_error());
        EXPECT_FALSE(renderer.add_vertex_buffer().has_error());
        EXPECT_FALSE(renderer.add_index_buffer().has_error());

        data.start_time = std::chrono::high_resolution_clock::now();
    }
    void update(float dt, AppInput& input, AppState& state, OneSecondAppData& data) {

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);

        if (duration > std::chrono::seconds(1)) {
            state.should_close = true;
        }
    }

    void draw(AppRenderer& renderer, OneSecondAppData& data) {
    }

    void stop() {
    }
};

TEST(BasicApps, one_second_app) {
    autoRunApp<OneSecondAppData, OneSecondAppLogic>("Test App: One Second ");
}

struct TriangleAppData {
    std::chrono::steady_clock::time_point start_time;
    constexpr static auto TRIANGLE_VERTICES = std::to_array({ -0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f });
    constexpr static auto TRIANGLE_INDICES = std::to_array<uint32_t>({ 0, 1, 2 });
};

struct TriangleAppLogic {
    void init(AppRenderer& renderer, TriangleAppData& data) {
        constexpr auto vert =
            "precision mediump float; "
            "layout(location = 0) in vec3 aPos;"
            "void main() {"
            "    gl_Position = vec4(aPos, 1.0);"
            "}"sv;

        constexpr auto frag =
            "precision mediump float; "
            "out vec4 FragColor;  "
            " void main()"
            " {"
            "     FragColor = vec4(1.0, 0.5, 0.2, 1.0); "
            " }"sv;
        renderer.add_shader(vert, frag);
        renderer.add_vertex_array(Renderer::VertexBufferLayout<float, uint32_t, glm::vec3>{});

        data.start_time = std::chrono::high_resolution_clock::now();
    }
    void update(float dt, AppInput& input, AppState& state, TriangleAppData& data) {

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);

        if (duration > std::chrono::seconds(1)) {
            state.should_close = true;
        }
    }

    void draw(AppRenderer& renderer, TriangleAppData& data) {
    }

    void stop() {
    }
};

TEST(BasicApps, triangle_app) {
    autoRunApp<TriangleAppData, TriangleAppLogic>("Test App: Triangle App");
}