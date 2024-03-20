//
// Created by konstantin on 20.03.24.
//

#include "gtest/gtest.h"


class TestSimpleHazard : public ::testing::Test {
public:
};

TEST_F(TestSimpleHazard, Simple) {
    ASSERT_TRUE(true);
}