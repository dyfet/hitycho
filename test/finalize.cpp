// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include "system.hpp"
#include "finalize.hpp"

#include <hpx/hpx_init.hpp>
#include <hpx/modules/futures.hpp>

#include <memory>
#include <atomic>
#include <cassert>

namespace {
std::atomic<bool> detach_flag{false};
} // namespace

// cppcheck-suppress constParameterReference
auto hpx_main(hpx::program_options::variables_map& args) -> int { // NOLINT
    hpx::latch l(2);
    {
        auto async_guard = hitycho::util::make_detach([&] {
            detach_flag.store(true, std::memory_order_relaxed);
            l.count_down(1);
        });
    }
    l.arrive_and_wait();
    assert(detach_flag.load(std::memory_order_relaxed) && "detach_scope did not run"); //
    return hpx::finalize();
}

auto main(int argc, char *argv[]) -> int {
    return hpx::init(argc, argv);
}
