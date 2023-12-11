#include "Models/Staging.hpp"
#include "Platform.hpp"

#include <chrono>
#include <iostream>
#include <span>
#include <string_view>

auto print_then_quit = [](std::string& error) {
    std::cerr << error
              << std::endl;
};

auto print_num_faces = [](Models::Staging::Model& model) {
    std::cout << "Triangle Count: "
              << model.index_data.size() / 3
              << std::endl;
};

void timeLoadModel(const std::string& filePath) {
    auto start = std::chrono::high_resolution_clock::now();
    Models::Staging::loadModel(filePath).on_error(print_then_quit).on_value(print_num_faces);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Loading " << filePath << " took " << duration.count() << " milliseconds." << std::endl;
}

int main() {
    timeLoadModel("assets/test.fbx");
    return 0;
}