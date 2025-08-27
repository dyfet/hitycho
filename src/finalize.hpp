// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include "common.hpp"

#include <hpx/async.hpp>
#include <hpx/functional/invoke.hpp>
#include <tuple>
#include <utility>

namespace hitycho::util {
template <typename Func, typename... Args>
class defer_scope final {
public:
    static_assert(std::is_invocable_v<Func, Args...>, "Func must be invocable with Args...");

    explicit defer_scope(Func func, Args&&...args) noexcept((std::is_nothrow_constructible_v<Func, Func&&> && ... && std::is_nothrow_constructible_v<Args, Args&&>)) : func_(std::move(func)), args_(std::forward<Args>(args)...) {}

    ~defer_scope() noexcept(std::is_nothrow_invocable_v<Func, Args...>) {
        if constexpr (sizeof...(Args) == 0) {
            std::move(func_)();
        } else {
            std::apply([this](auto&&...a) {
                hpx::util::invoke(std::move(func_), std::forward<decltype(a)>(a)...);
            },
            std::move(args_));
        }
    }

    defer_scope(const defer_scope&) = delete;
    defer_scope(defer_scope&&) = delete;
    auto operator=(const defer_scope&) -> defer_scope& = delete;
    auto operator=(defer_scope&&) -> defer_scope& = delete;

private:
    Func func_;
    std::tuple<Args...> args_;
};

template <typename Func, typename... Args>
class detach_scope final {
public:
    static_assert(std::is_invocable_v<Func, Args...>, "Func must be invocable with Args...");

    explicit detach_scope(Func func, Args&&...args) noexcept((std::is_nothrow_constructible_v<Func, Func&&> && ... && std::is_nothrow_constructible_v<Args, Args&&>)) : func_(std::move(func)), args_(std::forward<Args>(args)...) {}

    ~detach_scope() noexcept {
        hpx::async([f = std::move(func_), args = std::move(args_)]() mutable {
            if constexpr (sizeof...(Args) == 0) {
                std::move(f)();
            } else {
                std::apply([&](auto&&...a) {
                    hpx::util::invoke(std::move(f), std::forward<decltype(a)>(a)...);
                },
                std::move(args));
            }
        });
    }

    detach_scope(const detach_scope&) = delete;
    detach_scope(detach_scope&&) = delete;
    auto operator=(const detach_scope&) -> detach_scope& = delete;
    auto operator=(detach_scope&&) -> detach_scope& = delete;

private:
    Func func_;
    std::tuple<Args...> args_;
};

template <typename Func, typename... Args>
auto make_detach(Func&& func, Args&&...args) {
    return detach_scope<std::decay_t<Func>, std::decay_t<Args>...>(
    std::forward<Func>(func), std::forward<Args>(args)...);
}

template <typename Func, typename... Args>
auto make_defer(Func&& func, Args&&...args) {
    return defer_scope<std::decay_t<Func>, std::decay_t<Args>...>(
    std::forward<Func>(func), std::forward<Args>(args)...);
}
} // namespace hitycho::util
