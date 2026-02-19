#include <gtest/gtest.h>

#include "plane.hpp"

using V = Vector<double>;
using P = Point<double>;
using L = Line<double>;

TEST(Plane, VectorArithmetic) {
    V ij{1, 1};

    EXPECT_EQ(-ij, V(-1, -1));

    EXPECT_EQ(ij + V(1, -1), V(2, 0));
    EXPECT_EQ(ij - V(1, -1), V(0, 2));

    EXPECT_EQ(ij * 3, V(3, 3));
    EXPECT_EQ(7 * ij, V(7, 7));

    EXPECT_EQ(ij / 2, V(0.5, 0.5));

    EXPECT_TRUE(V(1, 2).collinearWith(V(-2, -4)));
    EXPECT_FALSE(V(1, 2).collinearWith(V(2, 1)));

    EXPECT_TRUE(V(2, 1).orthogonalTo(V(-1, 2)));

    EXPECT_TRUE(V(3, 5).orthogonalTo(V(3, 5).orthogonal()));

    EXPECT_TRUE(V(3, 7).normalized().isNormalized());
}

TEST(Plane, Lines) {
    L l1{P{1, 2}, V{-1, 1}};
    L l2{P{1, -7}, V{0, 1}};

    EXPECT_EQ(l1.intersectionWith(l2), P(1, 2));

    L l3{P{0, 0}, P{1, 1}};
    EXPECT_FALSE(l1.intersectionWith(l3).has_value());

    L l4{P{4, 0}, V{-1, 0}};
    L l5{P{-2, 0}, V{5, 0}};
    EXPECT_EQ(l4, l5);

    EXPECT_EQ(l2.perpendicularAt(P(1, 0)), l4);
    EXPECT_EQ(l2.perpendicularAt(P(1, 0)), l5);
}
