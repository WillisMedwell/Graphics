#pragma once

#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <print>

#include "Util/StaticVector.hpp"

namespace TStaticVector {
    static size_t num_allocations;
    static size_t num_frees;

}
void* operator new(std::size_t size) {
    ++TStaticVector::num_allocations;
    return std::malloc(size);
}
void* operator new[](std::size_t size) {
    ++TStaticVector::num_allocations;
    return std::malloc(size);
}
void operator delete(void* ptr) noexcept {
    ++TStaticVector::num_frees;
    return std::free(ptr);
}
void operator delete[](void* ptr) noexcept {
    ++TStaticVector::num_frees;
    return std::free(ptr);
}

TEST(StaticVector, runtime_push_pop) {
    Util::StaticVector<int, 10> s_vector;
    s_vector.push_back(0);
    s_vector.push_back(1);
    s_vector.push_back(2);

    EXPECT_EQ(s_vector.size(), 3);
    EXPECT_EQ(s_vector[0], 0);
    EXPECT_EQ(s_vector[1], 1);
    EXPECT_EQ(s_vector[2], 2);

    s_vector.pop_back();
    s_vector.pop_back();

    EXPECT_EQ(s_vector.size(), 1);
    EXPECT_EQ(s_vector[0], 0);

    s_vector.pop_back();

    EXPECT_EQ(s_vector.size(), 0);
}

TEST(StaticVector, runtime_construction) {
    using namespace std::literals;
    auto allocs = TStaticVector::num_allocations;
    auto frees = TStaticVector::num_frees;
    {
        Util::StaticVector<std::string, 10> s_vector { "Hello"s, "World"s, "!"s };
        EXPECT_EQ(s_vector[0], "Hello"s);
        EXPECT_EQ(s_vector[1], "World"s);
        EXPECT_EQ(s_vector[2], "!"s);
        EXPECT_EQ(s_vector.size(), 3);
        s_vector.pop_back();
        s_vector.emplace_back("hi there");
    }
    allocs = TStaticVector::num_allocations - allocs;
    frees = TStaticVector::num_frees - frees;
    // ensure no memory leaks.
    EXPECT_EQ(allocs, frees);
}

TEST(StaticVector, runtime_erase) {
    using namespace std::literals;
    auto allocs = TStaticVector::num_allocations;
    auto frees = TStaticVector::num_frees;
    {
        Util::StaticVector<std::string, 10> s_vector { "Hello"s, "World"s, "!"s };
        EXPECT_EQ(s_vector[0], "Hello"s);
        EXPECT_EQ(s_vector[1], "World"s);
        EXPECT_EQ(s_vector[2], "!"s);
        EXPECT_EQ(s_vector.size(), 3);
        s_vector.erase(1);
        s_vector.emplace_back("hi there");
    }
    allocs = TStaticVector::num_allocations - allocs;
    frees = TStaticVector::num_frees - frees;
    // ensure no memory leaks.
    EXPECT_EQ(allocs, frees);
}