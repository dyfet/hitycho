// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include "system.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <dirent.h>

#if defined(__OpenBSD__)
#define stat64 stat   // NOLINT
#define fstat64 fstat // NOLINT
#endif

namespace hitycho::fsys {
using namespace std::filesystem;
using dirent_t = struct dirent *;

class dir_t final {
public:
    dir_t() = default;
    dir_t(const dir_t& from) = delete;
    dir_t(dir_t&& from) noexcept : dir_(std::exchange(from.dir_, nullptr)) {}
    explicit dir_t(int handle) noexcept : dir_(fdopendir(handle)) {}
    explicit dir_t(const std::string& path) noexcept : dir_(opendir(path.c_str())) {}

    ~dir_t() { release(); }

    operator bool() const noexcept { return is_open(); }
    auto operator!() const noexcept { return !is_open(); }
    auto operator*() noexcept -> dirent_t { return dir_ ? readdir(dir_) : nullptr; }
    auto operator=(const dir_t& from) -> dir_t& = delete;

    auto operator=(dir_t&& from) noexcept -> dir_t& {
        release();
        dir_ = std::exchange(from.dir_, nullptr);
        return *this;
    }

    auto operator=(const std::string& path) noexcept -> dir_t& {
        release();
        dir_ = opendir(path.c_str());
        return *this;
    }

    auto operator=(int handle) noexcept -> dir_t& {
        release();
        dir_ = fdopendir(handle);
        return *this;
    }

    auto is_open() const noexcept -> bool { return dir_ != nullptr; }

    auto get() noexcept -> dirent_t { return dir_ ? readdir(dir_) : nullptr; }

private:
    DIR *dir_{nullptr};

    void release() {
        if (dir_)
            ::closedir(std::exchange(dir_, nullptr));
    }
};
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

template <typename Func>
inline auto scan_prefix(const std::string& path, Func func) {
    std::size_t count = 0;
    fsys::dir_t dir(path);
    fsys::dirent_t entry{nullptr};
    while ((entry = dir.get()) != nullptr) {
        func(entry);
    }
    return count;
}
} // namespace hitycho
