#pragma once

#include <variant>

#include "Util/Concepts.hpp"

namespace Util {
    template <typename V, typename E>
    class Result
    {
    private:
        using Storage = std::variant<V, E>;

        Storage _result;

    public:
        constexpr Result(const V& value) noexcept {
            static_assert(!std::is_same_v<V, E>, "The Value Type and Error Type cannot be the same");
            static_assert(HasCopyConstructor<V>, "The Value type has no copy constructor.");
            if constexpr (HasCopyConstructor<E> && HasCopyConstructor<V>) {
                _result = Storage { value };
            }
        }
        constexpr Result(V&& value) noexcept {
            static_assert(HasMoveConstructor<V>, "The Value type has no move constructor.");
            if constexpr (HasMoveConstructor<E> && HasMoveConstructor<V>) {
                _result = Storage { std::forward<V>(value) };
            }
        }

        constexpr Result(const E& error) noexcept {
            static_assert(HasCopyConstructor<E>, "The Error type has no copy constructor.");
            if constexpr (HasCopyConstructor<E> && HasCopyConstructor<V>) {
                _result = Storage { error };
            }
        }
        constexpr Result(E&& error) noexcept {
            static_assert(HasMoveConstructor<E>, "The Error type has no move constructor.");
            if constexpr (HasMoveConstructor<E> && HasMoveConstructor<V>) {
                _result = Storage { std::forward<E>(error) };
            }
        }

        constexpr Result(const Result<V, E>& other) noexcept
            : _result(other._result) {
        }
        constexpr Result(Result<V, E>&& other) noexcept
            : _result(std::forward<std::variant<V, E>>(other._result)) { }

        constexpr auto operator=(const V& value) noexcept -> Result& {
            _result = value;
            return *this;
        }
        constexpr auto operator=(V&& value) noexcept -> Result& {
            _result = std::move(value);
            return *this;
        }
        constexpr auto operator=(const E& error) noexcept -> Result& {
            _result = error;
            return *this;
        }
        constexpr auto operator=(E&& error) noexcept -> Result& {
            _result = std::move(error);
            return *this;
        }
        constexpr auto operator=(const Result& result) noexcept -> Result& {
            this->_result = result._result;
            return *this;
        }
        constexpr auto operator=(Result&& result) noexcept -> Result& {
            std::swap(this->_result, result._result);
            return *this;
        }

        [[nodiscard]] constexpr auto has_value() noexcept -> bool {
            return std::holds_alternative<V>(_result);
        }
        [[nodiscard]] constexpr auto has_error() noexcept -> bool {
            return std::holds_alternative<E>(_result);
        }

        [[nodiscard]] constexpr auto value() -> V& {
            return std::get<V>(_result);
        }
        [[nodiscard]] constexpr auto error() -> E& {
            return std::get<E>(_result);
        }

        template <typename Pred>
        constexpr auto on_error(Pred pred) noexcept -> Result& {
            if (has_error()) {
                pred(error());
            }
            return *this;
        }
        template <typename Pred>
        constexpr auto on_value(Pred pred) noexcept -> Result& {
            if (has_value()) {
                pred(value());
            }
            return *this;
        }

        template <typename PredForValue, typename PredForError>
        constexpr auto on_either(PredForValue pred_for_value, PredForError pred_for_error) noexcept -> Result& {
            if (has_value()) {
                pred_for_value(value());
            } else {
                pred_for_error(error());
            }
            return *this;
        }
    };

} // Util
