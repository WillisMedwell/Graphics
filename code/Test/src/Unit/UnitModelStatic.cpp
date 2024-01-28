#include "Model/Model.hpp"
#include "Utily/Utily.hpp"
#include <gtest/gtest.h>

#include <App.hpp>
#include <iostream>

using namespace std::literals;

TEST(Unit, Model_static) {
    const std::filesystem::path cube_path = "./assets/cube.obj";
    EXPECT_TRUE(std::filesystem::exists(cube_path));

    auto maybe_data = Utily::FileReader::load_entire_file(cube_path);

    EXPECT_FALSE(maybe_data.has_error());

    auto maybe_cube = Model::decode_as_static_model(
        std::span {
            maybe_data.value().begin(),
            maybe_data.value().end() },
        { '.', 'o', 'b', 'j' });

    EXPECT_FALSE(maybe_cube.has_error());
    //EXPECT_EQ(maybe_cube.value().vertices.size(), 36);
}
