// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include "system.hpp"
#include "locking.hpp"

#include <array>

using namespace hitycho;

struct test {
    int v1{2};
    // int v2{7};
};

namespace {
lock::exclusive<std::unordered_map<std::string, std::string>> mapper;
lock::exclusive<int> counter(3);
lock::shared<std::unordered_map<std::string, std::string>> tshared;
lock::shared<struct test> testing;
lock::shared<std::array<int, 10>> tarray;
} // namespace

// cppcheck-suppress constParameterReference
auto hpx_main(hpx::program_options::variables_map& args) -> int { // NOLINT
    using namespace hitycho::lock;
    try {
        {
            exclusive_ptr map(mapper);
            assert(map->empty());
            map["here"] = "there";
            assert(map->size() == 1);
            assert(map["here"] == "there");
        }
        {
            writer_ptr map(tshared);
            map["here"] = "there";
        }
        {
            const reader_ptr map(tshared);
            assert(map.at("here") == "there");
        }
        {
            writer_ptr map(tarray);
            map[2] = 17;
        }
        {
            const reader_ptr map(tarray);
            assert(map[2] == 17);
        }

        exclusive_ptr count(counter);
        assert(*count == 3);
        ++*count;
        assert(*count == 4);
        count.unlock();

        exclusive_guard fixed(counter);
        assert(*fixed == 4);
        {
            writer_ptr modtest(testing);
            ++modtest->v1;
        }
        const reader_ptr<struct test> tester(testing);
        assert(tester->v1 == 3);
    } catch (...) {
        hpx::finalize();
        std::quick_exit(-1);
    }
    return hpx::finalize();
}

auto main(int argc, char *argv[]) -> int {
    return hpx::init(argc, argv);
}
