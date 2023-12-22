#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <format>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <ranges>
#include <source_location>
#include <span>
#include <type_traits>
#include <variant>

namespace Util {

    template <typename T, size_t S>
    class StaticVector
    {
    private:
#ifndef NDEBUG
        static constexpr bool IS_SIZE_CHECKED = true;
#else
        static constexpr bool IS_SIZE_CHECKED = false;
#endif
        size_t _size;
        alignas(T) std::byte _data[sizeof(std::array<T, S>)];

        static_assert(sizeof(_data) == sizeof(std::array<T, S>));

        inline void growth_check() {
            if constexpr (IS_SIZE_CHECKED) {
                if (_size > S) {
                    throw std::length_error("Exceeded fixed capacity");
                }
            }
        }
        inline void shrink_check() {
            if constexpr (IS_SIZE_CHECKED) {
                if (_size == 0) {
                    throw std::length_error("Cannot shrink");
                }
            }
        }

    public:
        constexpr StaticVector() {
            _size = 0;
        }

        constexpr StaticVector(StaticVector&&) = default;

        [[nodiscard]] constexpr auto inline size() const noexcept { return _size; }

        [[nodiscard]] auto inline data() noexcept -> T* { return reinterpret_cast<T*>(&_data[0]); }
        [[nodiscard]] auto inline data() const noexcept -> const T* { return reinterpret_cast<const T*>(&_data[0]); }

        [[nodiscard]] auto inline cbegin() const noexcept -> const T* { return data(); }
        [[nodiscard]] auto inline cend() const noexcept -> const T* { return std::next(cbegin(), _size); }

        [[nodiscard]] auto inline begin() noexcept -> T* { return data(); }
        [[nodiscard]] auto inline begin() const noexcept -> const T* { return cbegin(); }
        [[nodiscard]] auto inline end() noexcept -> T* { return std::next(begin(), _size); }
        [[nodiscard]] auto inline end() const noexcept -> const T* { return cend(); }

        [[nodiscard]] auto inline as_span() noexcept -> std::span<T> { return std::span<T> { data(), _size }; }
        [[nodiscard]] auto inline as_span() const noexcept -> const std::span<const T> { return std::span<const T> { cbegin(), cend() }; }

        [[nodiscard]] auto inline front() -> T& {
            if constexpr (IS_SIZE_CHECKED) {
                if (_size == 0) {
                    throw std::runtime_error("Cannot get front element of empty container.");
                }
            }
            return *begin();
        }
        [[nodiscard]] auto inline front() const -> const T& {
            if constexpr (IS_SIZE_CHECKED) {
                if (_size == 0) {
                    throw std::runtime_error("Cannot get front element of empty container.");
                }
            }
            return *cbegin();
        }

        [[nodiscard]] auto inline back() -> T& {
            if constexpr (IS_SIZE_CHECKED) {
                if (_size == 0) {
                    throw std::runtime_error("Cannot get back element of empty container.");
                }
            }
            return *std::ranges::prev(end());
        }
        [[nodiscard]] auto inline back() const -> const T& {
            if constexpr (IS_SIZE_CHECKED) {
                if (_size == 0) {
                    throw std::runtime_error("Cannot get back element of empty container.");
                }
            }
            return *std::ranges::prev(end());
        }

        [[nodiscard]] auto inline operator[](size_t index) -> T& {
            if constexpr (IS_SIZE_CHECKED) {
                if (index >= _size) {
                    throw std::runtime_error("Out of bounds index");
                }
            }
            return *std::ranges::next(begin(), index);
        }
        [[nodiscard]] auto inline operator[](size_t index) const -> const T& {
            if constexpr (IS_SIZE_CHECKED) {
                if (index >= _size) {
                    throw std::runtime_error("Out of bounds index");
                }
            }
            return *std::ranges::next(begin(), index);
        }

        void inline erase(T* iter) {
            if constexpr (IS_SIZE_CHECKED) {
                int64_t index = std::distance(begin(), iter);
                if (index >= static_cast<int64_t>(_size)) {
                    throw std::runtime_error("Out of bounds iterator");
                }
            }
            std::copy(std::next(iter), end(), iter);
            --_size;
            std::destroy_at(end());
        }
        void inline erase(size_t index) {
            erase(std::next(begin(), index));
        }

        void push_back() {
            growth_check();
            std::construct_at(end());
            ++_size;
        }
        void push_back(const T& e) {
            growth_check();
            new (end()) T(e);
            ++_size;
        }
        void push_back(T&& e) {
            growth_check();
            std::construct_at(end(), std::forward<T>(e));
            ++_size;
        }
        template <typename... Args>
        void emplace_back(Args&&... args) {
            growth_check();
            std::construct_at(end(), std::forward<Args>(args)...);
            ++_size;
        }
        void pop_back() {
            shrink_check();
            --_size;
            std::destroy_at(end());
        }

        StaticVector(std::ranges::range auto& range) {
            using Underlying = std::decay_t<decltype(*range.begin())>;

            static_assert(std::is_same_v<Underlying, T> || std::is_convertible_v<Underlying, T>);

            if (std::is_same_v<Underlying, T>) {
                _size = std::min(range.size(), S);
                std::ranges::uninitialized_copy(
                    range.begin(), std::ranges::next(range.begin(), _size), this->begin(), std::ranges::next(this->begin(), _size));
            } else if (std::is_convertible_v<Underlying, T>) {
                _size = 0;
                for (const auto& e : range | std::views::take(S)) {
                    emplace_back(static_cast<T>(e));
                }
            }
        }
        constexpr StaticVector(std::ranges::range auto&& range) {
            using Underlying = std::decay_t<decltype(*range.begin())>;

            static_assert(std::is_same_v<Underlying, T> || std::is_convertible_v<Underlying, T>);

            if (std::is_same_v<Underlying, T>) {
                _size = std::min(range.size(), S);

                std::ranges::uninitialized_move(
                    range.begin(), std::ranges::next(range.begin(), _size), this->begin(), std::ranges::next(this->begin(), _size));
            } else if (std::is_convertible_v<Underlying, T>) {
                _size = 0;
                for (const auto& e : range | std::views::take(S)) {
                    emplace_back(static_cast<T>(e));
                }
            }
        }

        template <typename Arg>
        StaticVector(std::initializer_list<Arg>&& args)
            : StaticVector(std::ranges::subrange(args.begin(), args.end())) {
        }

        template <size_t S2>
        StaticVector(const StaticVector<T, S2>& other)
            : StaticVector(other.as_span()) {
        }
        ~StaticVector() { std::ranges::destroy(begin(), end()); }
    };
}