//
// Created by konstantin on 20.03.24.
//

#include "gtest/gtest.h"

#include <atomic>
#include <memory>
#include <thread>

#include <simple/hazard_ptr.h>
#include "runner.h"

namespace {

using namespace std::chrono_literals;
using namespace NHazard;

void RunReadOnly(uint32_t num_threads, std::atomic<int*>* value) {
    TimeRunner runner{1s};
    for (auto i = 0u; i < num_threads; ++i) {
        auto x = std::make_shared<int>(0);
        auto func = [&, x]() mutable {
            auto* ptr = Acquire(value);
            *x |= *ptr;
            Release();
        };
        TaskWithExit task{std::move(func), UnregisterThread};
        runner.DoWithInit(RegisterThread, std::move(task));
    }
    ASSERT_TRUE(runner.Wait() < 25ns);
}

void RunRareUpdates(uint32_t num_threads) {
    std::atomic value = new int{};
    std::jthread updater{[&](std::stop_token token) {
        RegisterThread();
        auto i = 0;
        while (!token.stop_requested()) {
            Retire(value.exchange(new int{++i}));
            std::this_thread::sleep_for(1ms);
        }
        UnregisterThread();
        ASSERT_TRUE(i > 500);
    }};
    RunReadOnly(num_threads, &value);
}

}  // namespace

class TestSimpleTimeHazard : public ::testing::Test {
   public:
};

TEST_F(TestSimpleTimeHazard, ReadOnly) {
    for (auto num_threads : {1, 2, 4, 8, 16}) {
        std::atomic value = new int{1023};
        RunReadOnly(static_cast<uint32_t>(num_threads), &value);
    }
}

TEST_F(TestSimpleTimeHazard, RareUpdates) {
    for (auto num_threads : {1, 2, 4, 8, 16}) {
        RunRareUpdates(static_cast<uint32_t>(num_threads));
    }
}
