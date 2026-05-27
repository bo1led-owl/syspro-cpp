#include <gtest/gtest.h>

#include <unordered_set>

#include "hashset.hpp"

namespace {
class Pair {
    int x_;
    int y_;

public:
    explicit Pair(int x, int y) : x_{x}, y_{y} {}

    int x() const {
        return x_;
    }
    int y() const {
        return y_;
    }

    bool operator==(const Pair& that) const {
        return x_ == that.x_ && y_ == that.y_;
    }
};

struct PairHasher {
    size_t operator()(const Pair& p) const {
        return p.x() * p.y();
    }
};

class CopyOnly {
    int x_;

public:
    CopyOnly(int x) : x_{x} {}

    CopyOnly(const CopyOnly& that) : x_{that.x_} {}
    CopyOnly(CopyOnly&&) = delete;

    CopyOnly& operator=(const CopyOnly& that) {
        x_ = that.x_;
        return *this;
    }
    CopyOnly& operator=(CopyOnly&&) = delete;

    bool operator==(const CopyOnly& that) const {
        return x_ == that.x_;
    }

    int x() const {
        return x_;
    }
};

struct CopyOnlyHasher {
    size_t operator()(const CopyOnly& c) const {
        return c.x();
    }
};

class MoveOnly {
    int x_;

public:
    MoveOnly(int x) : x_{x} {}

    MoveOnly(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& that) : x_{std::exchange(that.x_, -1)} {}

    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly& operator=(MoveOnly&& that) {
        std::swap(x_, that.x_);
        return *this;
    }

    bool operator==(const MoveOnly& that) const {
        return x_ == that.x_;
    }

    int x() const {
        return x_;
    }
};

struct MoveOnlyHasher {
    size_t operator()(const MoveOnly& c) const {
        return c.x();
    }
};
}  // namespace

TEST(HashSet, Basic) {
    HashSet<int> ints{3, 2, 4, 5, 1, 6};

    for (int i = 1; i <= 6; ++i)
        EXPECT_TRUE(ints.contains(i));
}

TEST(HashSet, Insertion) {
    HashSet<int> ints;

    ints.insert(3);
    ints.insert(1);
    ints.insert(4);

    EXPECT_EQ(*ints.find(3), 3);
    EXPECT_EQ(*ints.find(1), 1);
    EXPECT_EQ(*ints.find(4), 4);
}

TEST(HashSet, AFewRehashes) {
    HashSet<int> ints;

    for (int i = 0; i < 256; ++i) {
        ints.insert(i);
    }

    for (int i = 0; i < 256; ++i) {
        auto it = ints.find(i);
        ASSERT_TRUE(it != ints.end());
        EXPECT_EQ(*it, i);
    }
}

TEST(HashSet, Removal) {
    HashSet<int> ints;

    for (int i = 0; i < 256; ++i) {
        ints.insert(i);
    }

    for (int i = 0; i < 256; ++i) {
        auto it = ints.find(i);
        ASSERT_TRUE(it != ints.end());
        EXPECT_EQ(*it, i);
    }

    ints.erase(42);
    ints.erase(ints.find(43));

    for (int i = 0; i < 256; ++i) {
        if (i == 42)
            EXPECT_FALSE(ints.contains(42));
        else if (i == 43)
            EXPECT_FALSE(ints.contains(43));
        else {
            auto it = ints.find(i);
            ASSERT_TRUE(it != ints.end());
            EXPECT_EQ(*it, i);
        }
    }
}

TEST(HashSet, CopyOnlyItem) {
    HashSet<CopyOnly, CopyOnlyHasher> objs;

    std::vector<int> xs{42, 3, 4, 37, 8, 9, 17, 24, 55, 63, 7};
    for (int x : xs) {
        objs.insert(CopyOnly(x));
    }

    EXPECT_EQ(xs.size(), objs.size());
    for (int x : xs) {
        EXPECT_TRUE(objs.contains(CopyOnly(x)));
    }

    for (const CopyOnly& c : objs) {
        EXPECT_NE(std::ranges::find(xs, c.x()), xs.end());
    }

    objs.insert(CopyOnly(42));
    EXPECT_EQ(xs.size(), objs.size());
}

TEST(HashSet, MoveOnlyItem) {
    HashSet<MoveOnly, MoveOnlyHasher> objs;

    std::vector<int> xs{42, 3, 4, 37, 8, 9, 17, 24, 55, 63, 7};
    for (int x : xs) {
        objs.insert(MoveOnly(x));
    }

    EXPECT_EQ(xs.size(), objs.size());
    for (int x : xs) {
        EXPECT_TRUE(objs.contains(MoveOnly(x)));
    }

    for (const MoveOnly& c : objs) {
        EXPECT_NE(std::ranges::find(xs, c.x()), xs.end());
    }

    objs.insert(MoveOnly(42));
    EXPECT_EQ(xs.size(), objs.size());
}

TEST(HashSet, Copying) {
    HashSet<int> objs;

    std::unordered_set<int> xs{42, 3, 4, 37, 8, 9, 17, 24, 55, 63, 7};
    for (int x : xs) {
        objs.insert(x);
    }

    auto validate = [&xs](const auto& objs) {
        EXPECT_EQ(xs.size(), objs.size());
        for (int x : xs) {
            EXPECT_TRUE(objs.contains(x));
        }

        for (int x : objs) {
            EXPECT_TRUE(xs.contains(x));
        }
    };

    validate(objs);

    HashSet<int> objs2(objs);
    validate(objs2);

    HashSet<int> objs3;
    objs3 = objs;
    validate(objs3);
}

TEST(HashSet, Moving) {
    HashSet<int> objs;

    std::unordered_set<int> xs{42, 3, 4, 37, 8, 9, 17, 24, 55, 63, 7};
    for (int x : xs) {
        objs.insert(x);
    }

    auto validate = [&xs](const auto& objs) {
        EXPECT_EQ(xs.size(), objs.size());
        for (int x : xs) {
            EXPECT_TRUE(objs.contains(x));
        }

        for (int x : objs) {
            EXPECT_TRUE(xs.contains(x));
        }
    };

    validate(objs);

    HashSet<int> objs2(std::move(objs));
    validate(objs2);
    EXPECT_TRUE(objs.empty());

    HashSet<int> objs3;
    objs3 = std::move(objs2);
    validate(objs3);
    EXPECT_TRUE(objs2.empty());
}

TEST(HashSet, NonTrivialItem) {
    std::unordered_set<Pair, PairHasher> xs{
        Pair(3, 2),
        Pair(37, 14),
        Pair(42, 12),
    };

    auto validate = [&xs](const auto& objs) {
        EXPECT_EQ(xs.size(), objs.size());
        for (const auto& x : xs) {
            EXPECT_TRUE(objs.contains(x));
        }

        for (const auto& x : objs) {
            EXPECT_TRUE(xs.contains(x));
        }
    };

    HashSet<Pair, PairHasher> objs;
    objs.emplace(42, 12);
    objs.emplace(3, 2);
    objs.emplace(37, 14);

    validate(objs);
}
