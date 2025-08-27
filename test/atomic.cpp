// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include "system.hpp"
#include "atomic.hpp"

using namespace hitycho;

namespace {
void test_atomic_once() {
    const atomic::once_t once;
    assert(is(once));
    assert(!is(once));
    assert(!once);
}

void test_atomic_sequence() {
    atomic::sequence_t<uint8_t> bytes(3);
    assert(*bytes == 3);
    assert(static_cast<uint8_t>(bytes) == 4);
}

void test_atomic_dictionary() {
    atomic::dictionary_t<int, std::string> dict;
    dict.insert_or_assign(1, "one");
    dict.insert_or_assign(2, "two");
    assert(dict.find(1).value() == "one"); // NOLINT
    assert(dict.size() == 2);
    assert(dict.contains(2));
    dict.remove(1);
    assert(!dict.contains(1));
    assert(dict.size() == 1);
    dict.each([](const int& key, std::string& value) {
        assert(key == 2);
        assert(value == "two");
        value = "two two";
    });
    assert(dict.find(2).value() == "two two"); // NOLINT
}

void test_atomic_refs() {
    int value = 0;
    const atomic_ref<int> ref(value);

    ref.store(10);
    assert(ref.load() == 10);
    assert(value == 10);
    assert(ref.fetch_add(5) == 10);
    assert(ref.load() == 15);

    ref = 20;
    assert(ref == 20);
    assert(value == 20);

    int expected = 20;
    assert(ref.compare_exchange_strong(expected, 99));
    assert(ref == 99);
}
} // end namespace

// cppcheck-suppress constParameterReference
auto hpx_main(hpx::program_options::variables_map& args) -> int { // NOLINT
    test_atomic_once();
    test_atomic_sequence();
    test_atomic_dictionary();
    test_atomic_refs();
    return hpx::finalize();
}

auto main(int argc, char *argv[]) -> int {
    return hpx::init(argc, argv);
}
