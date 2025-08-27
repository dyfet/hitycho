// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include "system.hpp"
#include "expected.hpp"

using namespace hitycho;

namespace {
auto ret_error() -> expected<std::string, int> {
    return expected<std::string, int>(23);
}

auto ret_string() -> expected<std::string, int> {
    return expected<std::string, int>("hello");
}

void test_expected() {
    auto e1 = ret_error();
    auto e2 = ret_string();

    assert(e1.error() == 23);
    assert(e2.value() == "hello");
}
} // namespace

// cppcheck-suppress constParameterReference
auto hpx_main(hpx::program_options::variables_map& args) -> int { // NOLINT
    test_expected();
    return hpx::finalize();
}

auto main(int argc, char *argv[]) -> int {
    return hpx::init(argc, argv);
}
