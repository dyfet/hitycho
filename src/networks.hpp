// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include "sockets.hpp"

#include <ifaddrs.h>

namespace hitycho::socket {
using iface_t = struct ifaddrs *;

class networks {
public:
    using reference = struct ifaddrs&;
    using pointer = struct ifaddrs *;
    using const_reference = const struct ifaddrs&;
    using size_type = std::size_t;
    using value_type = struct ifaddrs;

    constexpr static struct ifaddrs *npos = nullptr;

    class iterator {
    public:
        using value_type = struct ifaddrs;
        using pointer = struct ifaddrs *;
        using reference = struct ifaddrs&;
        using difference_type = std::ptrdiff_t;
        using iterator_concept = std::forward_iterator_tag;

        constexpr iterator() = default;
        constexpr explicit iterator(pointer ptr) : ptr_(ptr) {}

        constexpr auto operator*() const -> reference { return *ptr_; }
        constexpr auto operator->() const -> pointer { return ptr_; }

        constexpr auto operator++() -> iterator& {
            ptr_ = ptr_ ? ptr_->ifa_next : nullptr;
            return *this;
        }

        constexpr auto operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        friend constexpr auto operator==(const iterator& lhs, const iterator& rhs) {
            return lhs.ptr_ == rhs.ptr_;
        }

        friend constexpr auto operator!=(const iterator& lhs, const iterator& rhs) {
            return !(lhs == rhs);
        }

    private:
        pointer ptr_{nullptr};
    };

    networks() { ::getifaddrs(&list_); }
    constexpr explicit networks(struct ifaddrs *list) : list_(list) {}
    constexpr networks(const networks& from) = delete;
    constexpr auto operator=(const networks& from) -> networks& = delete;
    networks(networks&& from) noexcept : list_(std::exchange(from.list_, nullptr)) {}
    ~networks() { release(); }

    constexpr explicit operator bool() const noexcept { return !empty(); }
    constexpr auto operator!() const noexcept { return empty(); }

    auto operator=(networks&& from) noexcept -> networks& {
        if (this->list_ == from.list_) return *this;
        release();
        list_ = std::exchange(from.list_, nullptr);
        return *this;
    }

    auto operator=(struct ifaddrs *addr) noexcept -> networks& {
        if (list_ != addr) {
            release();
            list_ = addr;
        }
        return *this;
    }

    constexpr auto first() const noexcept -> iface_t {
        return list_;
    }

    constexpr auto front() const noexcept -> iface_t {
        return list_;
    }

    constexpr auto empty() const noexcept -> bool { return list_ == nullptr; }
    constexpr auto begin() const noexcept { return iterator{list_}; }
    constexpr auto end() const noexcept { return iterator{nullptr}; } // NOLINT

    auto find(std::string_view id, int family = AF_UNSPEC, bool multicast = false) const noexcept -> iface_t {
        for (auto entry = list_; entry != nullptr; entry = entry->ifa_next) {
            if (multicast && !(entry->ifa_flags & IFF_MULTICAST)) continue;
            if (!entry->ifa_addr || !entry->ifa_name) continue;

            auto ifa_family = int(entry->ifa_addr->sa_family);
            if (family == AF_UNSPEC && (ifa_family == AF_INET || ifa_family == AF_INET6)) ifa_family = AF_UNSPEC;
            if (id == entry->ifa_name && family == ifa_family) return entry;
        }
        return nullptr;
    }

    auto find(const struct sockaddr *from) const noexcept -> iface_t {
        if (!from || !list_) return nullptr;
        for (auto entry = list_; entry != nullptr; entry = entry->ifa_next) {
            auto family = entry->ifa_addr->sa_family;
            if (!entry->ifa_addr || !entry->ifa_netmask) continue;
            if (family != from->sa_family) continue;
            if (family == AF_INET) {
                auto *t = in4_cast(from);
                auto *a = in4_cast(entry->ifa_addr);
                auto *m = in4_cast(entry->ifa_netmask);
                if ((a->sin_addr.s_addr & m->sin_addr.s_addr) == (t->sin_addr.s_addr & m->sin_addr.s_addr)) return entry;
            } else if (family == AF_INET6) {
                auto *a6 = in6_cast(entry->ifa_addr);
                auto *m6 = in6_cast(entry->ifa_netmask);
                auto *t6 = in6_cast(from);
                auto match = true;
                for (int i = 0; i < 16; ++i) {
                    if ((a6->sin6_addr.s6_addr[i] & m6->sin6_addr.s6_addr[i]) != (t6->sin6_addr.s6_addr[i] & m6->sin6_addr.s6_addr[i])) {
                        match = false;
                        break;
                    }
                }
                if (match) return entry;
            }
        }
        return nullptr;
    }

protected:
    iface_t list_{nullptr};

    void release() noexcept {
        if (list_) {
            ::freeifaddrs(list_);
            list_ = npos;
        }
    }
};
} // namespace hitycho::socket

namespace hitycho {
using networks_t = socket::networks;

inline auto bind_address(const networks_t& nets, const std::string& id, uint16_t port = 0, int family = AF_UNSPEC, bool multicast = false) -> address_t {
    address_t any;
    if (id == "[*]" && family == AF_UNSPEC) {
        any.family_if(AF_INET6);
        any.port(port);
        return any;
    }
    if (id == "*") {
        if (family == AF_UNSPEC)
            family = AF_INET;
        any.family_if(family);
        any.port(port);
        return any;
    }
    if ((family == AF_INET || family == AF_UNSPEC) && id.find('.') != std::string_view::npos)
        return socket::address::from_string(id, port);
    if ((family == AF_INET6 || family == AF_UNSPEC) && id.find(':') != std::string_view::npos)
        return socket::address::from_string(id, port);
    auto ifa = nets.find(id, family, multicast);
    if (ifa && ifa->ifa_addr) {
        socket::address a(ifa->ifa_addr);
        a.port(port);
        return a;
    }
    return any;
}

inline auto multicast_index(const networks_t& nets, const std::string& id, int family = AF_UNSPEC) -> unsigned {
    if (id == "*" && (family == AF_INET || family == AF_UNSPEC)) return ~0U;
    auto ifa = nets.find(id, family, true);
    if (ifa && ifa->ifa_addr) {
        if (ifa->ifa_addr->sa_family == AF_INET) return ~0U;
        return if_nametoindex(ifa->ifa_name);
    }
    return 0U;
}
} // namespace hitycho
