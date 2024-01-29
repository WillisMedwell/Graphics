#pragma once

#include "Renderer/BatchDrawer.hpp"
#include "Utily/Utily.hpp"
#include <gtest/gtest.h>

#include <App.hpp>
#include <iostream>
#include <string>

using namespace std::literals;

TEST(Unit, Renderer_batch_drawer_to_3_digits) {

    for (uint32_t i = 0; i <= 999; ++i) {
        auto result = Renderer::BatchDrawer::to_3_digit(i);
        auto expected = std::to_string(i);
        while (expected.size() < 3) {
            expected = '0' + expected;
        }

        EXPECT_EQ(result[0], expected[0]);
        EXPECT_EQ(result[1], expected[1]);
        EXPECT_EQ(result[2], expected[2]);
    }
}
