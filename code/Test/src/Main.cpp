#include <gtest/gtest.h>

#include "Unit/StaticVector.hpp"
#include "Integration/BasicApp.hpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}