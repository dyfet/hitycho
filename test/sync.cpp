// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#undef NDEBUG
#include "system.hpp"
#include "sync.hpp"
#include "pipeline.hpp"

using namespace hitycho;

namespace {
void test_sync_event() {
    std::atomic<bool> fin{false};
    sync::event done;
    const hpx::jthread thr([&] {
        hpx::this_thread::sleep_for(std::chrono::milliseconds(120));
        fin = true;
        done.signal();
    });
    done.wait();
    assert(fin == true);
}

void test_sync_barrier() {
    hpx::barrier<> bar(2);
    std::atomic<bool> completed{false};
    {
        const sync::barrier_scope guard(bar);
        (void)bar.arrive();
    }

    completed.store(true); // manually trigger completion
    assert(completed.load());
}

void test_sync_semaphore() {
    sync::semaphore<8> sem(2);
    sem.acquire();
    sem.acquire();
    assert(!sem.try_acquire() && "Semaphore should be exhausted");

    sem.release();
    assert(sem.try_acquire() && "Semaphore should allow reacquire");

    sem.release();
    sem.release();
}

void test_sync_waitgroup() {
    sync::wait_group wg(1);
    {
        const sync::group_scope done(wg);
        assert(wg.count() == 1);
    }
    assert(wg.count() == 0);
}
} // end namespace

// cppcheck-suppress constParameterReference
auto hpx_main(hpx::program_options::variables_map& args) -> int { // NOLINT
    test_sync_event();
    test_sync_barrier();
    test_sync_semaphore();
    test_sync_waitgroup();
    return hpx::finalize();
}

auto main(int argc, char *argv[]) -> int {
    return hpx::init(argc, argv);
}
