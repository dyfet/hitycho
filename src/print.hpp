// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include "system.hpp"

#include <sstream>

#include <hpx/hpx_finalize.hpp>
#include <hpx/iostream.hpp>
#include <hpx/format.hpp>

namespace hitycho {
inline constexpr auto GENERIC_DATETIME = "%c";
inline constexpr auto LOCAL_DATETIME = "%x %X";
inline constexpr auto ZULU_TIMESTAMP = "%Y-%m-%dT%H:%M:%SZ";
inline constexpr auto ISO_TIMESTAMP = "%Y-%m-%d %H:%M:%S %z";
inline constexpr auto ISO_DATETIME = "%Y-%m-%d %H:%M:%S";
inline constexpr auto ISO_DATE = "%Y-%m-%d";
inline constexpr auto ISO_TIME = "%X";

template <class... Args>
inline void print(std::string_view fmt, Args&&...args) {
    hpx::cout << hpx::util::format(fmt, std::forward<Args>(args)...);
}

template <typename Stream, class... Args>
inline auto print(Stream& out, std::string_view fmt, Args&&...args) -> Stream& {
    out << hpx::util::format(fmt, std::forward<Args>(args)...);
    return out;
}

class output final : public std::ostringstream {
public:
    class exit final : public std::ostringstream {
    public:
        exit(const exit&) = delete;
        auto operator=(const exit&) -> exit& = delete;

        explicit exit(int code) noexcept : exit_code_(code) {}
        [[noreturn]] ~exit() final {
            hpx::cerr << str() << std::endl
                      << std::ends;
            hpx::finalize(); // only if runtime is active
            std::quick_exit(exit_code_);
        }

    private:
        int exit_code_{-1};
    };

    class debug final : public std::ostringstream {
    public:
        debug(const debug&) = delete;
        auto operator=(const debug&) -> debug& = delete;

        debug() = default;
        ~debug() final {
#ifndef NDEBUG
            hpx::cout << str() << std::endl;
#endif
        }
    };

    class error final : public std::ostringstream {
    public:
        error(const error&) = delete;
        auto operator=(const error&) -> auto& = delete;

        error() = default;
        ~error() final {
            hpx::cerr << str() << std::endl;
        }
    };

    class null final : public std::ostream, private std::streambuf {
    public:
        null() : std::ostream(this) {}

        null(const null&) = delete;
        auto operator=(const null&) -> null& = delete;

    private:
        auto overflow(int c) -> int override {
            return c;
        }

        auto xsputn([[maybe_unused]] const char *s, std::streamsize n) -> std::streamsize override {
            return n;
        }
    };

    output() = default;
    output(const output&) = delete;
    explicit output(unsigned nl) : nl_(nl) {}
    auto operator=(const output&) -> output& = delete;

    ~output() final {
        hpx::cout << str();
        if (nl_)
            hpx::cout << std::string(nl_, '\n');
    }

private:
    unsigned nl_{1};
};

inline auto to_string(const std::tm& current, const char *fmt = ISO_DATETIME) {
    std::stringstream text;
    text << std::put_time(&current, fmt);
    return text.str();
}

inline auto gmt_datetime(const std::time_t& gmt) {
    std::stringstream text;
    auto current = system::gmt_time(gmt);
    text << std::put_time(&current, ZULU_TIMESTAMP);
    return text.str();
}

inline auto iso_datetime(const std::tm& current) {
    std::stringstream text;
    text << std::put_time(&current, ISO_DATETIME);
    return text.str();
}

inline auto iso_datetime(const std::time_t& current) {
    return iso_datetime(system::local_time(current));
}

inline auto iso_date(const std::tm& current) {
    return iso_datetime(current).substr(0, 10);
}

inline auto iso_date(const std::time_t& current) {
    return iso_datetime(current).substr(0, 10);
}

inline auto iso_time(const std::tm& current) {
    return iso_datetime(current).substr(11, 8);
}

inline auto iso_time(const std::time_t& current) {
    return iso_datetime(current).substr(11, 8);
}
} // namespace hitycho
