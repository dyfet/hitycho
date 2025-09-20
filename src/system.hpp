// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include "strings.hpp"
#include "timer.hpp"

#include <chrono>
#include <memory>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <hpx/hpx_init.hpp>

namespace hitycho::system {
using timepoint = hpx::chrono::steady_clock::time_point;
using duration = hpx::chrono::steady_clock::duration;
using args_t = std::vector<std::string>;

class notify_t final {
public:
    notify_t(const notify_t& from) = delete;

    notify_t() noexcept {
#ifdef EFD_NONBLOCK
        pipe_[0] = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (pipe_[0] == -1) return;
        pipe_[1] = pipe_[0];
#else
        if (pipe(pipe_) == -1) return;
        fcntl(pipe_[0], F_SETFL, O_NONBLOCK);
        fcntl(pipe_[1], F_SETFL, O_NONBLOCK);
#endif
    }

    ~notify_t() { release(); }

    auto is_open() const noexcept { return pipe_[0] != -1; }
    auto handle() const noexcept { return pipe_[0]; }

    auto clear() noexcept {
        if (pipe_[0] == -1) return false;
#ifdef EFD_NONBLOCK
        uint64_t count{0};
        const auto rtn = int(::read(pipe_[0], &count, sizeof(count)));
#else
        char buf[64]{};
        const auto rtn = int(::read(pipe_[0], buf, sizeof(buf)));
#endif
        if (rtn < 0) {
            release();
            return false;
        }
        return rtn > 0;
    }

    auto wait(int timeout = -1) noexcept {
        if (pipe_[0] == -1) return false;
        struct pollfd pfd = {.fd = pipe_[0], .events = POLLIN};
        auto rtn = ::poll(&pfd, 1, timeout);
        if (rtn < 0) {
            release();
            return false;
        }
        return rtn > 0;
    }

    auto signal() noexcept {
        if (pipe_[0] == -1) return false;
#ifdef EFD_NONBLOCK
        uint64_t one = 1;
        const auto rtn = int(::write(pipe_[1], &one, sizeof(one)));
#else
        const auto rtn = int(::write(pipe_[1], "x", 1));
#endif
        if (rtn < 0) {
            release();
            return false;
        }
        return rtn > 0;
    }

    auto operator=(const notify_t& from) -> notify_t& = delete;
    operator int() const noexcept { return pipe_[0]; } // select / poll

private:
    int pipe_[2]{-1, -1};

    void release() noexcept {
        if (pipe_[0] == -1) return;
        if (pipe_[0] != pipe_[1]) ::close(pipe_[1]);
        ::close(pipe_[0]);
        pipe_[0] = pipe_[1] = -1;
    }
};

inline auto make_argv(const args_t& args) {
    auto argv = std::make_unique<char *[]>(args.size() + 1);
    for (auto pos = 0U; pos < args.size(); ++pos)
        argv[pos] = const_cast<char *>(args[pos].c_str());
    argv[args.size() + 1] = nullptr;
    return argv;
}

inline void time_of_day(struct timeval *tp) noexcept {
    gettimeofday(tp, nullptr);
}

inline auto steady_time() noexcept {
    return hpx::chrono::steady_clock::now();
}

inline auto is_expired(const timepoint& deadline) {
    auto now = steady_time();
    return deadline < now;
}

inline auto put_timeval(struct timeval *tv, const timepoint& deadline) {
    using namespace hpx::chrono;
    if (tv == nullptr) return false;

    tv->tv_sec = tv->tv_usec = 0;
    auto now = steady_clock::now();
    if (deadline < now) return false;

    auto delta = deadline - now;
    auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
    tv->tv_sec = usecs / 1'000'000;
    tv->tv_usec = usecs % 1'000'000;
    return true;
}

inline auto get_timeout(const timepoint& deadline) -> int {
    using namespace hpx::chrono;
    auto now = steady_clock::now();
    auto dif = deadline > now ? deadline - now : steady_clock::duration::zero();
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dif).count();
    return int(std::min(int(msec), std::numeric_limits<int>::max()));
}

inline auto local_time(const std::time_t& time) noexcept {
    std::tm local{};
    localtime_r(&time, &local);
    return local;
}

inline auto gmt_time(const std::time_t& time) noexcept {
    std::tm local{};
    gmtime_r(&time, &local);
    return local;
}

inline auto is_dir(const std::string& path) noexcept -> bool {
    struct stat ino{};
    if (stat(path.c_str(), &ino))
        return false;

    if (S_ISDIR(ino.st_mode))
        return true;

    return false;
}

inline auto is_file(const std::string& path) noexcept -> bool {
    struct stat ino{};
    if (stat(path.c_str(), &ino))
        return false;

    if (S_ISREG(ino.st_mode))
        return true;

    return false;
}
}; // namespace hitycho::system

namespace hitycho {
using close_t = void (*)(int);
using timepoint_t = system::timepoint;
using duration_t = system::duration;

class handle_t final {
public:
    handle_t() = default;

    handle_t(handle_t&& from) noexcept : handle_(std::exchange(from.handle_, -1)), exit_(from.exit_) {}

    explicit handle_t(close_t fn) noexcept : exit_(fn) {}
    explicit handle_t(int handle) noexcept : handle_(handle) {}
    explicit handle_t(int handle, close_t fn) noexcept : handle_(handle), exit_(fn) {}
    ~handle_t() { close(); }

    handle_t(const handle_t&) = delete;
    auto operator=(const handle_t&) -> handle_t& = delete;

    operator int() const noexcept { return handle_; }
    explicit operator bool() const noexcept { return handle_ > -1; }
    auto operator!() const noexcept { return handle_ < 0; }

    auto operator=(int handle) noexcept -> handle_t& {
        close();
        handle_ = handle;
        return *this;
    }

    auto operator=(handle_t&& other) noexcept -> handle_t& {
        if (this == &other) return *this;
        close();
        handle_ = std::exchange(other.handle_, -1);
        exit_ = other.exit_;
        return *this;
    }

    auto get() const noexcept { return handle_; }
    auto is_open() const noexcept { return handle_ > -1; }
    auto release() noexcept { return std::exchange(handle_, -1); }
    auto clone() const noexcept { return dup(handle_); }

    void close() noexcept {
        if (handle_ != -1)
            exit_(std::exchange(handle_, -1));
    }

private:
    int handle_{-1};
    close_t exit_{[](int fd) { ::close(fd); }};
};

inline auto make_handle(const std::string& path, int mode, int perms = 0664) {
    return handle_t(::open(path.c_str(), mode, perms));
}
} // namespace hitycho
