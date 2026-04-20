#include <gtest/gtest.h>

#include "prime.hpp"

namespace {
bool isPrime(size_t n) {
    if (n < 2) {
        return false;
    }

    for (size_t k = 2; k * k <= n; ++k) {
        if (n % k == 0)
            return false;
    }

    return true;
}

size_t nthPrime(size_t n) {
    size_t cur = 2;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = cur + 1;; ++j) {
            if (isPrime(j)) {
                cur = j;
                break;
            }
        }
    }

    return cur;
}
}  // namespace

#define test(N) EXPECT_EQ(nthPrime(N), NthPrime<N>::value)

TEST(IsPrime, SomeNumbers) {
    test(0);
    test(1);
    test(2);
    test(3);
    test(4);
    test(5);
    test(6);
    // test(7) exceeds template instantiation depth limit
}
