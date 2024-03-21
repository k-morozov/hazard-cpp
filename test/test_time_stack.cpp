//
// Created by konstantin on 21.03.24.
//

#include <simple/hazard_ptr.h>
#include <stack/stack.h>

#include <array>
#include <utility>

#include "gtest/gtest.h"
#include "runner.h"

using namespace std::chrono_literals;
using namespace NHazard;
using namespace NSync;

class TestTimeStack : public ::testing::Test {
public:
};

TEST_F(TestTimeStack, StressPush) {
    Stack<int> stack;
    TimeRunner runner{1s};
    for (auto i = 0; i < 8; ++i) {
        TaskWithExit task{[&] { stack.Push(0); }, UnregisterThread};
        runner.DoWithInit(RegisterThread, std::move(task));
    }
    ASSERT_TRUE(runner.Wait() < 100ns);
}

static void StressPushPop(uint32_t num_push_threads, uint32_t num_pop_threads) {
    Stack<int> stack;

    TimeRunner push_runner{1s};
    for (auto i = 0u; i < num_push_threads; ++i) {
        TaskWithExit task{[&] { stack.Push(0); }, UnregisterThread};
        push_runner.DoWithInit(RegisterThread, std::move(task));
    }

    TimeRunner pop_runner{1s};
    for (auto i = 0u; i < num_pop_threads; ++i) {
        auto func = [&, value = 0]() mutable { stack.Pop(&value); };
        TaskWithExit task{std::move(func), UnregisterThread};
        pop_runner.DoWithInit(RegisterThread, std::move(task));
    }

    ASSERT_TRUE(push_runner.Wait() < 200ns);
    ASSERT_TRUE(pop_runner.Wait() < 10us);
}

TEST_F(TestTimeStack, StressPushAndPop) {
    std::vector<std::pair<uint32_t, uint32_t>> tests = {
        {42, 8}, {75, 5}, {15, 5}, {50, 10}};
    for (auto [num_push_threads, num_pop_threads] : tests) {
        StressPushPop(num_push_threads, num_pop_threads);
    }
}
