#pragma once

#include <concepts>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace Util {
    template <typename T>
    concept HasMoveConstructor = requires(T value, T&& move) {
        {
            new (&value) T(std::forward<T>(move))
        } -> std::same_as<T*>;
    };
    static_assert(HasMoveConstructor<std::unique_ptr<int>>);

    template <typename T>
    concept HasCopyConstructor = requires(T value, const T& copy) {
        {
            new (&value) T(copy)
        } -> std::same_as<T*>;
    };
    static_assert(HasCopyConstructor<int>);
    static_assert(!HasCopyConstructor<std::unique_ptr<int>>);

    template <typename T, typename E>
    concept IsRangeOf = requires(T container, E element) {
        {
            *container.begin()
        } -> std::same_as<E&>;
    };
    static_assert(IsRangeOf<std::vector<int>, int>);
    static_assert(!IsRangeOf<std::vector<int>, bool>);

    template <typename T, typename E>
    concept IsContiguousRangeOf = requires(T container, E element) {
        {
            *container.begin()
        } -> std::same_as<E&>;
        {
            container.data()
        } -> std::same_as<E*>;
    };
    static_assert(IsContiguousRangeOf<std::vector<int>, int>);
    static_assert(!IsContiguousRangeOf<std::list<int>, int>);

    template <typename T>
    concept HasIterators = requires(T container) {
        {
            container.begin()
        };
        {
            container.end()
        };
    };
    static_assert(HasIterators<std::vector<int>>);
    static_assert(!HasIterators<int[20]>);

} // namespace Util
