#include <gtest/gtest.h>

#include "hashset.hpp"

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
        EXPECT_EQ(*ints.find(i), i);
    }
}

TEST(HashSet, Removal) {
    HashSet<int> ints;

    for (int i = 0; i < 256; ++i) {
        ints.insert(i);
    }

    for (int i = 0; i < 256; ++i) {
        EXPECT_EQ(*ints.find(i), i);
    }

    ints.erase(42);
    ints.erase(ints.find(43));

    for (int i = 0; i < 256; ++i) {
        if (i == 42)
            EXPECT_FALSE(ints.contains(42));
        else if (i == 43)
            EXPECT_FALSE(ints.contains(43));
        else
            EXPECT_EQ(*ints.find(i), i);
    }
}
