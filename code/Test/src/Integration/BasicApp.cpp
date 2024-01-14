#include <gtest/gtest.h>

#include <App.hpp>

struct OneSecondAppData {
    std::chrono::steady_clock::time_point start_time;
};
struct OneSecondAppLogic {
    void init(OneSecondAppData& data [[maybe_unused]]) {
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

TEST(BasicApp, one_second_app) {
    autoRunApp<OneSecondAppData, OneSecondAppLogic>();
}