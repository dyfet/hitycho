// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include <variant>

namespace hitycho {
template <typename T, typename E>
class expected {
public:
    expected() = default;
    expected(const expected&) = default;
    explicit expected(const T& value) : value_(value) {}
    explicit expected(const E& error) : value_(error) {}

    auto operator=(const expected&) -> expected& = default;

    auto operator*() const -> const T& {
        return value();
    }

    auto operator*() -> T& {
        return value();
    }

    auto operator->() -> T * {
        return &(value());
    }

    auto operator->() const -> const T * {
        return &(value());
    }

    explicit operator bool() const {
        return has_value();
    }

    auto operator!() const {
        return !has_value();
    }

    auto has_value() const {
        return std::holds_alternative<T>(value_);
    }

    auto value() -> T& {
        return std::get<T>(value_);
    }

    auto value() const -> const T& {
        return std::get<T>(value_);
    }

    auto value_or(T& alt) -> T& {
        if (has_value()) return value();
        return alt;
    }

    auto error() const -> const E& {
        return std::get<E>(value_);
    }

private:
    std::variant<T, E> value_;
};
} // namespace hitycho
