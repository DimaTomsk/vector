#include "gtest/gtest.h"

#include "vector.h"

TEST(Vector, SimpleOps) {
    vector<int> x(1, 1);
    for (size_t i = 0; i < 5; ++i) {
        x.push_back(i + 2);
    }
    for (size_t i = 0; i < 3; ++i) {
        x.pop_back();
    }

    EXPECT_EQ(x, (vector{1, 2, 3}));
}

TEST(Vector, Iterators) {
    vector<int> x = {1, 2, 3, 4, 5};
    auto a = x.begin();
    auto b = x.end();

    EXPECT_EQ(*a, 1);
    EXPECT_EQ(b - a, 5);
    ++a;
    EXPECT_EQ(b - a, 4);
    a += 2;
    EXPECT_EQ(b - a, 2);
    EXPECT_EQ(b - a - 2, 0);
    EXPECT_EQ(b - a + 2, 4);
    a -= 2;
    EXPECT_EQ(b - a, 4);
    --a;
    EXPECT_EQ(b - a, 5);
    EXPECT_EQ(a + 5, b);
}

TEST(Vector, ReverseIterators) {
    vector<int> x = {1, 2, 3, 4, 5};
    auto a = x.rbegin();
    auto b = x.rend();

    EXPECT_EQ(*a, 5);
    EXPECT_EQ(b - a, 5);
    ++a;
    EXPECT_EQ(b - a, 4);
    a += 2;
    EXPECT_EQ(b - a, 2);
    EXPECT_EQ(b - a - 2, 0);
    EXPECT_EQ(b - a + 2, 4);
    a -= 2;
    EXPECT_EQ(b - a, 4);
    --a;
    EXPECT_EQ(b - a, 5);
    EXPECT_EQ(a + 5, b);
}

TEST(Vector, BigTest) {
    vector<int> x = {1, 2, 3, 4, 5};
    vector<int> y = x;
    vector<int> z = std::move(x);

    EXPECT_EQ(x, (vector<int>{}));
    EXPECT_EQ(y, (vector<int>{1, 2, 3, 4, 5}));
    EXPECT_EQ(z, (vector<int>{1, 2, 3, 4, 5}));

    x = z;
    y = std::move(z);

    EXPECT_EQ(z, (vector<int>{}));
    EXPECT_EQ(y, (vector<int>{1, 2, 3, 4, 5}));
    EXPECT_EQ(x, (vector<int>{1, 2, 3, 4, 5}));

    x = {1, 2, 3};
    EXPECT_EQ(x, (vector<int>{1, 2, 3}));

    x.resize(5);
    EXPECT_EQ(x, (vector<int>{1, 2, 3, 0, 0}));

    x.resize(4);
    EXPECT_EQ(x, (vector<int>{1, 2, 3, 0}));

    x.resize(6, 6);
    EXPECT_EQ(x, (vector<int>{1, 2, 3, 0, 6, 6}));

    x.assign(3, 3);
    EXPECT_EQ(x, (vector<int>{3, 3, 3}));

    x.assign(y.begin(), y.end());
    EXPECT_EQ(x, (vector<int>{1, 2, 3, 4, 5}));
    EXPECT_EQ(x.at(1), 2);
    EXPECT_EQ(x.front(), 1);
    EXPECT_EQ(x.back(), 5);
    EXPECT_EQ(*x.data(), 1);

    x.clear();

    x.shrink_to_fit();

    EXPECT_EQ(x.empty(), true);
    EXPECT_EQ(x.size(), 0);
    EXPECT_EQ(x.capacity(), 0);
    EXPECT_LE(x, y);
}
