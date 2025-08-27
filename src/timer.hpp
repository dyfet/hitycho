// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include "common.hpp"

#include <hpx/hpx.hpp>
#include <hpx/async.hpp>
#include <hpx/thread.hpp>

#include <chrono>

namespace hitycho::timer {
namespace detail {
template <typename F, typename... Args>
void timeout_worker(std::chrono::microseconds delay, F&& callback, Args&&...args) {
    hpx::this_thread::sleep_for(delay);
    std::invoke(std::forward<F>(callback), std::forward<Args>(args)...);
}

template <typename F, typename... Args>
void interval_worker(std::chrono::microseconds interval, std::shared_ptr<std::atomic<bool>> cancelled, F&& callback, Args&&...args) {
    using clock = std::chrono::steady_clock;
    auto next = clock::now() + interval;
    while (!cancelled->load(std::memory_order_relaxed)) {
        hpx::this_thread::sleep_until(next);
        if (cancelled->load(std::memory_order_relaxed)) break;
        std::invoke(std::forward<F>(callback), std::forward<Args>(args)...);
        next += interval;
    }
}
} // namespace detail

using cancel_token = std::shared_ptr<std::atomic<bool>>;
using time_period = std::chrono::microseconds;

class cancel_guard {
public:
    explicit cancel_guard(cancel_token&& token) noexcept : cancel_token_(std::move(token)) {}

    cancel_guard(cancel_guard&& other) noexcept : cancel_token_(std::exchange(other.cancel_token_, nullptr)) {}

    auto operator=(cancel_guard&& other) noexcept -> cancel_guard& {
        if (this == &other) return *this;
        cancel_token_ = std::exchange(other.cancel_token_, nullptr);
        return *this;
    }

    cancel_guard(const cancel_guard&) = delete;
    auto operator=(const cancel_guard&) -> cancel_guard& = delete;

    ~cancel_guard() {
        if (cancel_token_) {
            cancel_token_->store(true, std::memory_order_release);
            cancel_token_ = nullptr; // consume the token...
        }
    }

private:
    cancel_token cancel_token_;
};

inline auto make_token() -> cancel_token {
    return std::make_shared<std::atomic<bool>>(false);
}

inline auto release_token(cancel_token& token) {
    if (token) {
        token->store(true, std::memory_order_relaxed);
        token = nullptr; // consume the token...
    }
}

template <typename F, typename... Args>
void once(time_period delay, F&& callback, Args&&...args) {
    hpx::async(detail::timeout_worker<F, Args...>, delay, std::forward<F>(callback), std::forward<Args>(args)...);
}

template <typename F, typename... Args>
auto periodic(time_period interval, F&& callback, Args&&...args) {
    auto cancelled = make_token();
    hpx::async(detail::interval_worker<F, Args...>, interval, cancelled, std::forward<F>(callback), std::forward<Args>(args)...);
    return cancelled;
}

// a timer group can share a single cancellation
template <typename F, typename... Args>
void periodic(time_period interval, cancel_token& cancel, F&& callback, Args&&...args) {
    hpx::async(detail::interval_worker<F, Args...>, interval, cancel, std::forward<F>(callback), std::forward<Args>(args)...);
}
} // namespace hitycho::timer
