// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include "system.hpp"
#include "timer.hpp"

#include <cassert>

// cppcheck-suppress constParameterReference
auto hpx_main(hpx::program_options::variables_map& args) -> int { // NOLINT
    std::atomic<bool> flag{false};

    hitycho::timer::once(std::chrono::milliseconds(500), [&] {
        flag.store(true, std::memory_order_relaxed);
    });

    hitycho::timer::once(std::chrono::milliseconds(1000), [&] {
        assert(flag.load(std::memory_order_relaxed) && "Flag should have been set!");
    });

    hpx::this_thread::sleep_for(std::chrono::milliseconds(1500));
    return hpx::finalize();
}

// cppcheck-suppress constParameterReference
auto main(int argc, char *argv[]) -> int {
    return hpx::init(argc, argv);
}
