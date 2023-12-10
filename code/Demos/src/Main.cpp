#include "Platform.hpp"
#include "Staging.hpp"

#include <iostream>
#include <span>
#include <string_view>

auto print_then_quit = [](std::string& error) {
    std::cerr << error
              << '\n'
              << std::flush;
};

int main() {
    Staging::loadModel("assets/test.txt").on_error(print_then_quit);
    Staging::loadModel("assets/test.fbx").on_error(print_then_quit);
    Staging::loadModel("assets/test.glb").on_error(print_then_quit);
    Staging::loadModel("assets/test.gltf").on_error(print_then_quit);

    return 0;
}