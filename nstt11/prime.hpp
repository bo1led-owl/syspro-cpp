#pragma once

#include <cstddef>
#include <type_traits>

template <size_t N, size_t K = 2>
struct IsPrime : public std::conditional_t<
                     (K * K > N),
                     std::true_type,
                     std::conditional_t<N % K == 0, std::false_type, IsPrime<N, K + 1>>> {};

template <size_t N, size_t P = 2>
struct NthPrime
    : public std::conditional_t<
          IsPrime<P>::value,
          std::conditional_t<N == 0, std::integral_constant<size_t, P>, NthPrime<N - 1, P + 1>>,
          NthPrime<N, P + 1>> {};
