// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include <hpx/functional.hpp>
#include <type_traits>
#include <stdexcept>
#include <utility>
#include <cstdint>

#ifndef HITYCHO_RUNTIME_SYSTEM
#define HITYCHO_RUNTIME_SYSTEM
#endif

namespace hitycho {
using error = std::runtime_error;
using range = std::out_of_range;
using invalid = std::invalid_argument;
using overflow = std::overflow_error;

template <typename T>
constexpr auto is(const T& object) {
    return static_cast<bool>(object);
}

template <typename T>
constexpr auto is_null(const T& ptr) {
    if constexpr (std::is_pointer_v<T>)
        return ptr == nullptr;
    else
        return !static_cast<bool>(ptr);
}

template <typename Func, typename Fallback, typename... Args>
auto try_function_impl(Func&& func, Fallback&& fallback, std::false_type is_void [[maybe_unused]], Args&&...args)
-> std::invoke_result_t<Func, Args...> {
    try {
        return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    } catch (...) {
        return std::forward<Fallback>(fallback);
    }
}

template <typename Func, typename Fallback, typename... Args>
auto try_function(Func&& func, Fallback&& fallback, Args&&...args) -> std::invoke_result_t<Func, Args...> {
    using is_void = std::is_void<std::invoke_result_t<Func, Args...>>;

    return try_function_impl(std::forward<Func>(func), std::forward<Fallback>(fallback), is_void{}, std::forward<Args>(args)...);
}
} // namespace hitycho

namespace hitycho::util {
template <typename T = void>
constexpr auto offset_ptr(void *base, std::size_t offset) -> T * {
    return reinterpret_cast<T *>(static_cast<std::uint8_t *>(base) + offset);
}

template <typename T>
constexpr auto is_within_bounds(const T *ptr, const T *base, std::size_t count) {
    return ptr >= base && ptr < base + count;
}

template <typename Range, typename T>
constexpr auto count(const Range& range, const T& value) noexcept -> std::size_t {
    std::size_t result = 0;
    for (const auto& element : range)
        // cppcheck-suppress useStlAlgorithm
        result += static_cast<std::size_t>(element == value);
    return result;
}

template <typename T>
constexpr auto pow(T base, T exp) {
    static_assert(std::is_integral_v<T>, "pow requires integral types");
    T result = 1;
    while (exp) {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}
} // namespace hitycho::util
