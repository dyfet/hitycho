// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include "system.hpp"
#include "scan.hpp"

using namespace hitycho;

namespace {
void test_hex_parsing() {
    assert(parse_hex("f0") == 240);
    assert(parse_hex<uint16_t>("fff0") == 65520);
    assert(parse_hex<uint16_t>("0xfff0") == 65520);
    assert(parse_hex<uint16_t>("$fff0") == 65520);
}

void test_int_parsing() {
    auto myint = parse_unsigned<uint16_t>("23");
    assert(myint == 23);
    assert(sizeof(myint) == 2); // NOLINT
}

void test_try_parsing() {
    std::string_view input = "-1";
    auto fallback = 42U;
    auto result = try_function([&] {
        return parse_unsigned<unsigned>(input);
    },
    fallback);

    assert(result == fallback);
}
} // end namespace

// cppcheck-suppress constParameterReference
auto hpx_main(hpx::program_options::variables_map& args) -> int { // NOLINT
    test_hex_parsing();
    test_int_parsing();
    test_try_parsing();
    return hpx::finalize();
}

auto main(int argc, char *argv[]) -> int {
    return hpx::init(argc, argv);
}
