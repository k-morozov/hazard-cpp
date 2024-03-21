//
// Created by konstantin on 21.03.24.
//
#include <simple/hazard_ptr.h>
#include <stack/stack.h>

#include <algorithm>
#include <atomic>
#include <random>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

using namespace std::chrono_literals;
using namespace NHazard;
using namespace NSync;

class TestStack : public ::testing::Test {
public:
};

TEST_F(TestStack, Order) {
    static constexpr auto kRange = std::views::iota(0, 10);
    RegisterThread();

    Stack<int> s;
    for (auto x : kRange) {
        s.Push(x);
    }

    int value{};
    for (auto x : kRange | std::views::reverse) {
        ASSERT_TRUE(s.Pop(&value));
        ASSERT_TRUE(value == x);
    }
    ASSERT_FALSE(s.Pop(&value));

    UnregisterThread();
}

TEST_F(TestStack, Destructor) {
    RegisterThread();

    Stack<std::vector<int>> s;
    s.Push({1, 2, 3});

    UnregisterThread();
}

TEST_F(TestStack, Clear) {
    RegisterThread();

    std::vector v = {1, 2, 3};
    Stack<std::vector<int>> s;
    s.Push(v);
    s.Push(v);
    s.Clear();
    ASSERT_FALSE(s.Pop(&v));

    s.Clear();
    ASSERT_FALSE(s.Pop(&v));

    UnregisterThread();
}

TEST_F(TestStack, Pushes) {
    static constexpr auto kNumThreads = 16;
    static constexpr auto kRange = std::views::iota(0, kNumThreads);
    static constexpr auto kN = 10'000 * kNumThreads;
    RegisterThread();

    Stack<int> s;
    auto push_f = [&](int init) {
        RegisterThread();
        for (auto i = init; i < kN; i += kNumThreads) {
            s.Push(i);
        }
        UnregisterThread();
    };

    std::vector<std::jthread> threads;
    for (auto i : kRange) {
        threads.emplace_back(push_f, i);
    }
    threads.clear();

    std::array<int, kNumThreads> lasts;
    std::ranges::copy(std::views::iota(kN, kN + kNumThreads), lasts.begin());
    size_t count = 0;
    for (int value; s.Pop(&value); ++count) {
        auto& last = lasts[value % kNumThreads];
        ASSERT_TRUE(value == last - kNumThreads);
        last = value;
    }
    ASSERT_TRUE(count == kN);
    ASSERT_TRUE(std::ranges::equal(lasts, kRange));

    UnregisterThread();
}

TEST_F(TestStack, Pops) {
    static constexpr auto kNumThreads = 16;
    static constexpr auto kN = 10'000 * kNumThreads;
    RegisterThread();

    std::mutex mutex;
    Stack<int> s;
    for (auto i : std::views::iota(0, kN)) {
        s.Push(i);
    }

    std::array<int, kN> used{};
    auto pop_f = [&] {
        RegisterThread();
        int value;
        for (auto last = kN; s.Pop(&value); last = value) {
            std::lock_guard guard{mutex};
            ASSERT_TRUE(value >= 0);
            ASSERT_TRUE(value < last);
            ++used[value];
        }
        UnregisterThread();
    };

    std::vector<std::jthread> threads;
    for (auto i = 0; i < kNumThreads; ++i) {
        threads.emplace_back(pop_f);
    }
    threads.clear();
    ASSERT_TRUE(std::ranges::count(used, 1) == kN);

    UnregisterThread();
}

template <class T>
std::vector<T> Merge(const std::vector<std::vector<T>>& data) {
    auto joined = std::views::join(data);
    return {joined.begin(), joined.end()};
}

TEST_F(TestStack, PushPop) {
    static constexpr auto kNumPushThreads = 6;
    static constexpr auto kNumPopThreads = 2;
    static constexpr auto kNumIterations = 10'000;
    RegisterThread();

    std::vector<std::vector<std::string>> pushed(kNumPushThreads);
    std::vector<std::jthread> threads;
    Stack<std::string> stack;
    std::atomic push_finished = 0;

    for (auto i = 0; i < kNumPushThreads; ++i) {
        threads.emplace_back([&, i] {
            RegisterThread();
            std::mt19937 gen{3675475u * (i + 1)};
            std::uniform_int_distribution dist{'a', 'z'};
            auto gen_string = [&] {
                std::string s(8, '\0');
                for (auto& c : s) {
                    c = dist(gen);
                }
                return s;
            };
            for (auto j = 0; j < kNumIterations; ++j) {
                auto str = gen_string();
                pushed[i].push_back(str);
                stack.Push(str);
            }
            ++push_finished;
            UnregisterThread();
        });
    }

    std::vector<std::vector<std::string>> popped(kNumPopThreads);
    for (auto i = 0; i < kNumPopThreads; ++i) {
        threads.emplace_back([&, &popped = popped[i]] {
            RegisterThread();
            std::string str;
            while (true) {
                if (stack.Pop(&str)) {
                    popped.push_back(str);
                } else if (push_finished == kNumPushThreads) {
                    break;
                }
            }
            UnregisterThread();
        });
    }

    threads.clear();
    auto all_pushed = Merge(pushed);
    auto all_popped = Merge(popped);
    std::ranges::sort(all_pushed);
    std::ranges::sort(all_popped);
    ASSERT_EQ(all_pushed, all_popped);

    UnregisterThread();
}
