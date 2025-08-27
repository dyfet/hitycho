// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#pragma once

#include "sockets.hpp"
#include "sync.hpp"

#include <hpx/future.hpp>

namespace hitycho::socket {
using name_t = std::pair<std::string, std::string>;
using addr_t = std::pair<const struct sockaddr *, socklen_t>;

class service {
public:
    using reference = struct addrinfo&;
    using pointer = struct addrinfo *;
    using const_reference = const struct addrinfo&;
    using size_type = std::size_t;
    using value_type = struct addrinfo;

    constexpr static struct addrinfo *npos = nullptr;

    class iterator {
    public:
        using value_type = struct addrinfo;
        using pointer = addrinfo *;
        using reference = addrinfo&;
        using difference_type = std::ptrdiff_t;
        using iterator_concept = std::forward_iterator_tag;

        constexpr iterator() = default;
        constexpr explicit iterator(pointer ptr) : ptr_(ptr) {}

        constexpr auto operator*() const -> reference { return *ptr_; }
        constexpr auto operator->() const -> pointer { return ptr_; }

        constexpr auto operator++() -> iterator& {
            ptr_ = ptr_ ? ptr_->ai_next : nullptr;
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

    constexpr service() = default;
    constexpr explicit service(struct addrinfo *list) : list_(list) {}
    constexpr service(const service& from) = delete;
    constexpr auto operator=(const service& from) -> service& = delete;
    service(service&& from) noexcept : list_(std::exchange(from.list_, nullptr)) {}
    ~service() { release(); }

    constexpr operator struct sockaddr *() noexcept {
        if (!list_) return nullptr;
        return list_->ai_addr;
    }

    constexpr operator const struct sockaddr *() noexcept {
        if (!list_) return nullptr;
        return list_->ai_addr;
    }

    constexpr operator addr_t() const noexcept { return addr(); }
    constexpr operator const struct addrinfo *() const noexcept { return list_; }
    constexpr explicit operator bool() const noexcept { return !empty(); }
    constexpr auto operator*() const noexcept { return list_; }
    constexpr auto operator!() const noexcept { return empty(); }

    auto operator=(service&& from) noexcept -> service& {
        if (this->list_ == from.list_) return *this;
        release();
        list_ = std::exchange(from.list_, nullptr);
        return *this;
    }

    constexpr auto operator=(struct addrinfo *addr) noexcept -> service& {
        if (list_ != addr) {
            release();
            list_ = addr;
        }
        return *this;
    }

    constexpr auto first() const noexcept -> const struct addrinfo * {
        return list_;
    }

    constexpr auto front() const noexcept -> const struct addrinfo * {
        return list_;
    }

    constexpr auto addr() const noexcept -> addr_t {
        if (!list_) return {};
        return {list_->ai_addr, list_->ai_addrlen};
    }

    constexpr auto c_sockaddr() const noexcept -> const struct sockaddr * {
        if (!list_) return nullptr;
        return list_->ai_addr;
    }

    constexpr auto empty() const noexcept -> bool { return list_ == nullptr; }
    constexpr auto begin() const noexcept { return iterator{list_}; }
    constexpr auto end() const noexcept { return iterator{nullptr}; } // NOLINT

    template <typename Func>
    auto count(Func func) const {
        auto visited = 0U;
        auto list = list_;
        while (list) {
            if (func(list))
                ++visited;
            list = list->ai_next;
        }
        return visited;
    }

    template <typename Func>
    auto find(Func func) const {
        auto list = list_;
        while (list) {
            if (func(list))
                return list;
            list = list->ai_next;
        }
        return npos;
    }

protected:
    struct addrinfo *list_{npos};

    void release() noexcept {
        if (list_) {
            ::freeaddrinfo(list_);
            list_ = npos;
        }
    }
};

inline auto lookup(const addr_t& info, int flags = 0) -> name_t {
    const auto& [addr, len] = info;
    if (!addr) return {"", ""};
    char hostbuf[256];
    char portbuf[64];
    if (!getnameinfo(addr, len, hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf), flags)) {
        return {hostbuf, portbuf};
    }
    return {"", ""};
}

inline auto lookup(const name_t& name = name_t("*", ""), int family = AF_UNSPEC, int type = SOCK_STREAM, int protocol = 0) -> service {
    const auto& [host, service] = name;
    struct addrinfo *list{nullptr};
    auto svc = service.c_str();
    if (service.empty() || service == "0")
        svc = nullptr;

    struct addrinfo hint{};
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = family;
    hint.ai_socktype = type;
    hint.ai_protocol = protocol;

    auto sock_port{0};
    try {
        if (svc) sock_port = std::stoi(service);
    } catch (...) {
        sock_port = 0;
    }

    if (sock_port > 0 && sock_port < 65536)
        hint.ai_flags |= NI_NUMERICSERV;

    auto addr = host.c_str();
    if (host.empty() || host == "*")
        addr = nullptr;
    else if (host == "[*]") {
        addr = nullptr;
        hint.ai_family = AF_INET6;
    } else if (strchr(addr, ':'))
        hint.ai_flags |= AI_NUMERICHOST;

    if (protocol)
        hint.ai_flags |= AI_PASSIVE;

    auto err = getaddrinfo(addr, svc, &hint, &list);
    if (err && list) {
        freeaddrinfo(list);
        list = nullptr;
    }
    return socket::service(list);
}

inline auto lookup(const address& addr, int flags = 0) -> name_t {
    return lookup(addr_t(addr.data(), addr.size()), flags);
}

inline auto from_addr(const address& addr) {
    return addr_t(addr.data(), addr.size());
}

inline auto from_host(const std::string& host) {
    return name_t(host, "");
}
} // namespace hitycho::socket

namespace hitycho {

class resolver_timeout final : public std::runtime_error {
public:
    resolver_timeout() noexcept : std::runtime_error("resolver timeout") {}
};

inline auto async_resolver(const socket::name_t& name, int family = AF_UNSPEC, int type = SOCK_STREAM, int protocol = 0) -> hpx::future<socket::service> {
    return hpx::async(hpx::launch::async, [name, family, type, protocol] {
        return socket::lookup(name, family, type, protocol);
    });
}

inline auto async_resolver(const socket::addr_t& info, int flags = 0) -> hpx::future<socket::name_t> {
    return hpx::async(hpx::launch::async, [info, flags] {
        return socket::lookup(info, flags);
    });
}

inline auto defer_resolver(const socket::name_t& name = socket::name_t("*", ""), int family = AF_UNSPEC, int type = SOCK_STREAM, int protocol = 0) -> hpx::future<socket::service> {
    return hpx::async(hpx::launch::deferred, [name, family, type, protocol] {
        return socket::lookup(name, family, type, protocol);
    });
}

inline auto defer_resolver(const socket::addr_t& info, int flags = 0) {
    return hpx::async(hpx::launch::deferred, [info, flags] {
        return socket::lookup(info, flags);
    });
}
} // namespace hitycho
