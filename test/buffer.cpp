// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include "system.hpp"
#include "buffer.hpp"

using namespace hitycho;

namespace {
void test_format() {
    format_buffer<32> output;
    output << "hi " << "there";
    assert(!strcmp(*output, "hi there"));
}
} // namespace

// cppcheck-suppress constParameterReference
auto hpx_main(hpx::program_options::variables_map& args) -> int { // NOLINT
    test_format();
    return hpx::finalize();
}

auto main(int argc, char *argv[]) -> int {
    return hpx::init(argc, argv);
}
