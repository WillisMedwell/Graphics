#pragma once

#include <gtest/gtest.h>

#include <App.hpp>

struct TwoSecondLogic {
    void init(AppData& data [[maybe_unused]]) {
    }
    void update(float dt [[maybe_unused]], AppData& data [[maybe_unused]], const AppInput& input [[maybe_unused]]) {
        static auto start_time = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time);

        if (duration > std::chrono::seconds(2)) {
            data.should_close = true;
        }
    }
    void stop() {
    }
};

TEST(BasicApp, two_second_app) {
    autoRunApp<TwoSecondLogic>();
}