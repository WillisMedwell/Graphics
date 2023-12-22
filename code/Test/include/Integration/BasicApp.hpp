#pragma once

#include <gtest/gtest.h>

#include <App.hpp>

struct TwoSecondLogic {
    void init(AppData& data) {
    }
    void update(float dt, AppData& data, const AppInput& input) {
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
    AutoRunApp(App<TwoSecondLogic>{});
}