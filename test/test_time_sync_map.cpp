//
// Created by konstantin on 22.03.24.
//
#include <map/sync_map.h>
#include <simple/hazard_ptr.h>

#include <atomic>
#include <shared_mutex>
#include <unordered_map>

#include "gtest/gtest.h"
#include "runner.h"

namespace {

using namespace std::chrono_literals;
using namespace NHazard;
using namespace NSync;

void RunSharedMutex(uint32_t num_threads) {
    std::unordered_map<int, int> map;
    std::shared_mutex mutex;
    for (auto i = 0; i < 1024; ++i) {
        map[i] = i;
    }
    TimeRunner runner{1s};
    auto is_error = false;
    for (auto i = 0u; i < num_threads; ++i) {
        auto func = [&, i, is_error = std::atomic_ref{is_error}] {
            if (std::shared_lock guard{mutex}; !map.contains(i)) {
                is_error.store(true, std::memory_order::relaxed);
            }
        };
        TaskWithExit task{std::move(func), UnregisterThread};
        runner.DoWithInit(RegisterThread, std::move(task));
    }
    ASSERT_TRUE(runner.Wait() >= 30ns);
    ASSERT_FALSE(is_error);
}

void RunSyncMap(uint32_t num_threads) {
    SyncMap<int, int> map;
    for (auto i = 0; i < 1024; ++i) {
        map.Insert(i, i);
    }
    TimeRunner runner{1s};
    auto is_error = false;
    for (auto i = 0u; i < num_threads; ++i) {
        auto func = [&, i, is_error = std::atomic_ref{is_error},
                     v = 0]() mutable {
            if (!map.Lookup(i, &v)) {
                is_error.store(true, std::memory_order::relaxed);
            }
        };
        TaskWithExit task{std::move(func), UnregisterThread};
        runner.DoWithInit(RegisterThread, std::move(task));
    }
    ASSERT_TRUE(runner.Wait() < 15ns);
    ASSERT_FALSE(is_error);
}

}  // namespace

class TestTimeSyncMap : public ::testing::Test {
public:
};

TEST_F(TestTimeSyncMap, SharedMutex) {
    for (auto num_threads : {4, 8, 16}) {
        RunSharedMutex(num_threads);
    }
}

TEST_F(TestTimeSyncMap, SyncMap) {
    for (auto num_threads : {4, 8, 16}) {
        RunSyncMap(num_threads);
    }
}
