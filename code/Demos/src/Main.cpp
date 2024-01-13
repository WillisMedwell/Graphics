#include "App.hpp"
#include "Models/Staging.hpp"
#include "Config.hpp"

#include <chrono>
#include <iostream>
#include <span>
#include <string_view>

#include "Util/StaticVector.hpp"

auto print_then_quit = [](std::string& error) {
    std::cerr << error
              << std::endl;
};

auto print_num_faces = [](Models::Staging::Model& model) {
    std::println("Triangle Count: {}", model.index_data.size() / 3);
};

void timeLoadModel(const std::string& file_path) {
    auto start = std::chrono::high_resolution_clock::now();
    Models::Staging::loadModel(file_path).on_error(print_then_quit).on_value(print_num_faces);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Loading " << file_path << " took " << duration.count() << " milliseconds." << std::endl;
}

struct Logic {
    void init(AppData& data) {
        timeLoadModel("assets/test.fbx");
    }
    void update(float dt, AppData& data, const AppInput& input) {
        data.should_close = true;
    }
    void stop() {
    }
};

int main() { 
    autoRunApp<Logic>();
    return 0;
}