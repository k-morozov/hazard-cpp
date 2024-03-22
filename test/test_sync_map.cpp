//
// Created by konstantin on 22.03.24.
//

#include <map/sync_map.h>
#include <simple/hazard_ptr.h>

#include <random>

#include "gtest/gtest.h"

using namespace std::chrono_literals;
using namespace NHazard;
using namespace NSync;

class TestSyncMap : public ::testing::Test {
public:
};

TEST_F(TestSyncMap, Simple) {
    RegisterThread();
    SyncMap<int, int> map;

    int value{};
    ASSERT_FALSE(map.Lookup(0, &value));

    ASSERT_TRUE(map.Insert(0, 42));
    ASSERT_TRUE(map.Lookup(0, &value));
    ASSERT_TRUE(value == 42);

    ASSERT_FALSE(map.Insert(0, 42));
    UnregisterThread();
}

TEST_F(TestSyncMap, UpgradeToReadOnly) {
    RegisterThread();
    SyncMap<int, int> map;

    map.Insert(0, 42);
    map.Insert(1, 42);
    map.Insert(2, 42);

    for (auto i = 0; i < 1024; ++i) {
        int value{};
        ASSERT_TRUE(map.Lookup(0, &value));
        ASSERT_TRUE(value == 42);
    }
    UnregisterThread();
}

TEST_F(TestSyncMap, Stress) {
    std::unordered_map<int, int> std_map;
    SyncMap<int, int> map;

    std::mt19937_64 gen{42};
    std::uniform_int_distribution dist{0, 100'000};

    for (auto i = 0; i < 1'000'000; ++i) {
        auto key = dist(gen);
        int value{};

        auto it = std_map.find(key);
        auto found = map.Lookup(key, &value);
        if (it == std_map.end()) {
            ASSERT_FALSE(found);
        } else {
            ASSERT_TRUE(found);
            ASSERT_TRUE(value == it->second);
        }

        if (!found && (dist(gen) % 2)) {
            value = dist(gen);
            map.Insert(key, value);
            std_map[key] = value;
        }
    }
}
