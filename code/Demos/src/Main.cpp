#include "App.hpp"
#include "Config.hpp"
#include "Models/Models.hpp"

#include <chrono>
#include <iostream>
#include <span>
#include <string_view>

#include <Utily/Utily.hpp>

// auto print_then_quit = [](auto& error) {
//     std::cerr
//         << error.what()
//         << std::endl;
//     exit(1);
// };

// auto print_num_faces = [](Model::Staging::Model& model) {
//     std::println("Triangle Count: {}", model.index_data.size() / 3);
// };

// void timeLoadModel(const std::string& file_path) {
//     auto start = std::chrono::high_resolution_clock::now();
//     Models::Staging::loadModel(file_path).on_error(print_then_quit).on_value(print_num_faces);
//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double, std::milli> duration = end - start;
//     std::cout << "Loading " << file_path << " took " << duration.count() << " milliseconds." << std::endl;
// }

struct Data {
    Cameras::StationaryPerspective camera {
        glm::vec3(0, 0, 0),
        glm::vec3(1, 0, 0),
        90.0f
    };
};

struct Logic {
    void init(AppRenderer& renderer, Data& data) {
    }
    void update(float dt, const AppInput& input, AppState& state, Data& data) {
        state.should_close = true;
    }
    void draw(AppRenderer& renderer, Data& data) {
        const auto proj_mat = data.camera.projection_matrix(renderer.window_width, renderer.window_height);
        const auto view_mat = data.camera.view_matrix();
    }
    void stop() {
    }
};

int main() {
    autoRunApp<Data, Logic>();
    return 0;
}