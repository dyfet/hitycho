// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include "system.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

#if defined(__OpenBSD__)
#define stat64 stat   // NOLINT
#define fstat64 fstat // NOLINT
#endif

namespace hitycho::fsys {
using namespace std::filesystem;

class file_t final {
public:
    explicit file_t(FILE *file) : file_(system::file_ptr(file)) {}
    explicit file_t(system::file_ptr&& file) : file_(std::move(file)) {}

    operator FILE *() const noexcept { return file_.get(); }
    explicit operator bool() const noexcept { return is_open(); }
    auto operator!() const noexcept { return !is_open(); }
    auto is_open() const noexcept -> bool { return file_.get() != nullptr; }

private:
    system::file_ptr file_{nullptr};
};

class pipe_t final {
public:
    explicit pipe_t(system::pipe_ptr&& pipe) : pipe_(std::move(pipe)) {}

    operator FILE *() const noexcept { return pipe_.get(); }
    explicit operator bool() const noexcept { return is_open(); }
    auto operator!() const noexcept { return !is_open(); }
    auto is_open() const noexcept -> bool { return pipe_.get() != nullptr; }

private:
    system::pipe_ptr pipe_{nullptr};
};

inline auto make_file(const std::string& path, const std::string& mode = "rb") {
    return file_t(system::file_ptr(fopen(path.c_str(), mode.c_str())));
}

inline auto make_pipe(const std::string& cmd, const std::string& mode = "r") {
    return pipe_t(system::make_pipe(cmd, mode));
}
} // namespace hitycho::fsys

namespace hitycho {
template <typename Func>
inline auto scan_stream(std::istream& input, Func func) {
    static_assert(std::is_invocable_v<Func, std::string>, "Func not callable");
    static_assert(std::is_convertible_v<std::invoke_result_t<Func, std::string>, bool>, "Result must be bool");

    std::string line;
    std::size_t count{0};
    while (std::getline(input, line)) {
        if (!func(line)) break;
        ++count;
    }
    return count;
}

template <typename Func>
inline auto scan_file(const fsys::path& path, Func func) {
    static_assert(std::is_invocable_v<Func, std::string>, "Func not callable");
    static_assert(std::is_convertible_v<std::invoke_result_t<Func, std::string>, bool>, "Result must be bool");

    std::fstream input(path);
    std::string line;
    std::size_t count{0};
    while (std::getline(input, line)) {
        if (!func(line)) break;
        ++count;
    }
    return count;
}

template <typename Func>
inline auto scan_command(const std::string& cmd, Func func, std::size_t size = 0) {
    auto pipe = system::make_pipe(cmd, "r");
    if (!pipe.get()) return std::size_t(0);

    auto count = scan_file(pipe.get(), func, size);
    return count;
}

template <typename Func>
inline auto scan_directory(const fsys::path& path, Func func) {
    using Entry = const fsys::directory_entry&;
    static_assert(std::is_invocable_v<Func, Entry>, "Func must be callable");
    static_assert(std::is_convertible_v<std::invoke_result_t<Func, Entry>, bool>, "Func must return bool");

    auto dir = fsys::directory_iterator(path);
    return std::count_if(begin(dir), end(dir), func);
}

template <typename Func>
inline auto scan_recursive(const fsys::path& path, Func func) {
    using Entry = const fsys::directory_entry&;
    static_assert(std::is_invocable_v<Func, Entry>, "Func must be callable");
    static_assert(std::is_convertible_v<std::invoke_result_t<Func, Entry>, bool>, "Func must return bool");

    auto dir = fsys::recursive_directory_iterator(path, fsys::directory_options::skip_permission_denied);
    return std::count_if(begin(dir), end(dir), func);
}
} // namespace hitycho
