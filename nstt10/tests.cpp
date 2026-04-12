#include <gtest/gtest.h>

#include "limiter.hpp"

namespace {
class TestClass : Limiter<TestClass, 4> {};

class MoveOnlyTestClass : Limiter<TestClass, 2> {
public:
    MoveOnlyTestClass() = default;

    MoveOnlyTestClass(const MoveOnlyTestClass&) = delete;

    MoveOnlyTestClass(MoveOnlyTestClass&&) = default;

    MoveOnlyTestClass& operator=(MoveOnlyTestClass&&) = default;
};
}  // namespace

TEST(Limiter, RegularConstructor) {
    TestClass t1;
    TestClass t2;
    TestClass t3;
    TestClass t4;
    ASSERT_THROW(TestClass t5, std::runtime_error);
    ASSERT_THROW(TestClass t6, std::runtime_error);
}

TEST(Limiter, CopyConstructor) {
    TestClass t1;
    TestClass t2(t1);
    TestClass t3(t1);
    TestClass t4(t2);
    ASSERT_THROW(TestClass t5(t3), std::runtime_error);
    ASSERT_THROW(TestClass t6(t4), std::runtime_error);
}

TEST(Limiter, MoveConstructor) {
    MoveOnlyTestClass t1;
    MoveOnlyTestClass t2(std::move(t1));
    ASSERT_THROW(MoveOnlyTestClass t5(std::move(t2)), std::runtime_error);
    ASSERT_THROW(MoveOnlyTestClass t6(std::move(t2)), std::runtime_error);
}

TEST(Limiter, CopyAssignment) {
    TestClass t1;
    TestClass t2;
    TestClass t3;
    TestClass t4;

    ASSERT_NO_THROW(t1 = t2);
    ASSERT_NO_THROW(t3 = t1);
    ASSERT_NO_THROW(t4 = t2);
}

TEST(Limiter, MoveAssignment) {
    MoveOnlyTestClass t1;
    MoveOnlyTestClass t2;

    ASSERT_NO_THROW(t1 = std::move(t2));
    ASSERT_NO_THROW(t2 = std::move(t1));
}
