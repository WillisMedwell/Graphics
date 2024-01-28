#include <gtest/gtest.h>

#include "Unit/UnitModelStatic.hpp"
#include "Integration/AssetLoading.hpp"
#include "Integration/BasicApps.hpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}