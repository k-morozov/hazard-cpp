//
// Created by konstantin on 20.03.24.
//

#include <simple/hazard_ptr.h>

#include <atomic>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

namespace detail {

struct State {
    explicit State(int value) : value{value} { ++num; }
    ~State() { --num; }
    State(const State&) = delete;
    State& operator=(const State&) = delete;

    static inline std::atomic num = 0;
    int value;
};

void Run(std::atomic<State*>* value) {
    static std::mutex mutex;
    static std::mutex mutex_check;
    static auto x = 0;

    NHazard::RegisterThread();

    auto last_read = 0;
    for (auto i = 0; i < 100'000; ++i) {
        if (i % 123 == 0) {
            State* old_value;
            {
                std::lock_guard guard{mutex};
                old_value = value->exchange(new State{++x});
            }
            NHazard::Retire(old_value);
        } else {
            if (auto* p = NHazard::Acquire(value)) {
                std::lock_guard guard{mutex_check};
                ASSERT_TRUE(p->value >= last_read);
                last_read = p->value;
            }
            NHazard::Release();
        }
    }
    NHazard::Retire(value->exchange(nullptr));

    NHazard::UnregisterThread();
}

}  // namespace detail

class TestSimpleHazard : public ::testing::Test {
   public:
};

TEST_F(TestSimpleHazard, SingleThread) {
    NHazard::RegisterThread();

    std::atomic value = new int{42};
    auto* p = NHazard::Acquire(&value);
    ASSERT_EQ(*p, 42);

    for (auto i = 0; i < 100; ++i) {
        NHazard::Retire(value.exchange(new int{100500}));
    }
    NHazard::Retire(value.exchange(nullptr));

    ASSERT_EQ(*p, 42);
    NHazard::Release();

    NHazard::UnregisterThread();
}

TEST_F(TestSimpleHazard, ManyThreads) {
    for (auto i = 0; i < 5; ++i) {
        std::atomic<detail::State*> value = nullptr;
        std::vector<std::jthread> threads;
        threads.reserve(10);
        for (auto j = 0; j < 10; ++j) {
            threads.emplace_back(detail::Run, &value);
        }
        threads.clear();
        ASSERT_EQ(detail::State::num, 0);
    }
}