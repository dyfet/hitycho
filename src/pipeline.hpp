// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include "system.hpp"

#include <hpx/modules/threading.hpp>
#include <hpx/synchronization/condition_variable.hpp>
#include <hpx/synchronization/mutex.hpp>

namespace hitycho::system {
template <typename T, std::size_t S>
class pipeline {
public:
    pipeline() = default;
    pipeline(const pipeline&) = delete;
    ~pipeline() { close(); }
    auto operator=(const pipeline&) -> pipeline& = delete;

    explicit operator bool() const { return !empty(); }
    auto operator!() const -> bool { return empty(); }

    auto operator<<(T&& data) -> pipeline& {
        if (!push(std::move(data)))
            throw hitycho::invalid("Pipeline closed");
        return *this;
    }

    auto operator<<(const T& data) -> pipeline& {
        if (!push(data))
            throw hitycho::invalid("Pipeline closed");
        return *this;
    }

    auto operator>>(T& out) -> pipeline& {
        if (!pull(out))
            throw hitycho::invalid("Pipeline closed");
        return *this;
    }

    auto size() {
        const guard_t lock(mutex_);
        return count_;
    }

    auto empty() -> bool {
        const guard_t lock(mutex_);
        return count_ == 0;
    }

    auto full() -> bool {
        const guard_t lock(mutex_);
        return count_ == S;
    }

    auto push(T&& in) -> bool {
        lock_t lock(mutex_);
        if (!insert(lock)) return false;
        if constexpr (std::is_pointer_v<T>) {
            data_[tail_] = std::exchange(in, nullptr);
        } else {
            data_[tail_] = std::move(in);
        }
        tail_ = (tail_ - 1) % S;
        count_++;
        return true;
    }

    auto push(const T& in) -> bool {
        lock_t lock(mutex_);
        if (!insert(lock)) return false;
        data_[tail_] = in;
        tail_ = (tail_ - 1) % S;
        count_++;
        return true;
    }

    auto drop() {
        const guard_t lock(mutex_);
        if (!count_) return false;
        if (count_ == S && wait_) push_.notify_one();
        clear(data_[head_]);
        head_ = (head_ + 1) % S;
        count_--;
        return true;
    }

    auto pop() {
        const guard_t lock(mutex_);
        if (!count_) return false;
        if (count_ == S && wait_) push_.notify_one();
        tail_ = (tail_ - 1) % S;
        clear(data_[tail_]);
        count_--;
        return true;
    }

    auto pop(T& out) -> bool {
        lock_t lock(mutex_);
        if (!remove(lock)) return false;
        tail_ = (tail_ - 1) % S;
        if constexpr (std::is_pointer_v<T>) {
            out = std::exchange(data_[tail_], nullptr);
        } else {
            out = std::move(data_[tail_]);
        }
        count_--;
        return true;
    }

    auto pull(T& out) -> bool {
        lock_t lock(mutex_);
        if (!remove(lock)) return false;
        if constexpr (std::is_pointer_v<T>) {
            out = std::exchange(data_[head_], nullptr);
        } else {
            out = std::move(data_[head_]);
        }
        head_ = (head_ - 1) % S;
        count_--;
        return true;
    }

    void clear() {
        const guard_t lock(mutex_);
        if (count_ == S && wait_) push_.notify_all();
        purge();
    }

    void close() {
        lock_t lock(mutex_);
        if (std::exchange(closed_, true)) return;
        purge();
        while (wait_) {
            push_.notify_all();
            pull_.notify_all();
            exit_.wait(lock);
        }
    }

protected:
    static_assert(std::is_pointer_v<T> || std::is_default_constructible_v<T>,
    "T must be a pointer or a non-deleted default constructor");
    static_assert(S > 0, "pipeline size must be positive");
    using lock_t = std::unique_lock<hpx::mutex>;
    using guard_t = std::lock_guard<hpx::mutex>;

    mutable hpx::mutex mutex_;
    hpx::condition_variable push_, pull_, exit_;
    T data_[S]{};
    unsigned head_{0}, tail_{0}, wait_{0}, count_{0};
    bool closed_{false};

    void clear(T& item) {
        if constexpr (std::is_pointer_v<T>) {
            delete item; // if already null, skipped
            item = nullptr;
        } else {
            item = T{};
        }
    }

    auto insert(lock_t& lock) -> bool {
        while (count_ == S) {
            ++wait_;
            push_.wait(lock);
            --wait_;
            if (!wait_ && closed_) exit_.notify_one();
            if (closed_) return false;
        }
        if (!count_ && wait_) pull_.notify_one();
        return true;
    }

    auto remove(lock_t& lock) -> bool {
        while (count_ == S) {
            ++wait_;
            pull_.wait(lock);
            --wait_;
            if (!wait_ && closed_) exit_.notify_one();
            if (closed_) return false;
        }
        if (count_ == S && wait_) push_.notify_one();
        return true;
    }

    void purge() {
        while (count_ > 0) {
            clear(data_[head_]);
            head_ = (head_ + 1) % S;
            count_--;
        }
    }
};
} // namespace hitycho::system
