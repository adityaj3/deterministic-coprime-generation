#ifndef COPRIME_ENGINE_HPP
#define COPRIME_ENGINE_HPP

#include <vector>
#include <utility>
#include <stdexcept>
#include <type_traits>
#include <concepts>
#include <cstdint>
#include <numeric>
#include <limits>
#include <cmath>
#include <boost/multiprecision/number.hpp>

namespace coprime {

    // ============================================================================
    // 1. META NAMESPACE: Type Resolution & SFINAE Protection
    // ============================================================================
    namespace meta {
        // Base template: Safely decays fundamental types without triggering eager evaluation
        template<typename T>
        struct evaluate_ast {
            using type = std::remove_cvref_t<T>;
        };

        // Concept-gated specialization: Safely evaluates Boost lazy expressions
        template<typename T>
        requires boost::multiprecision::is_number_expression<std::remove_cvref_t<T>>::value
        struct evaluate_ast<T> {
            using type = typename std::remove_cvref_t<T>::result_type;
        };

        // Standardized access alias
        template<typename T>
        using eval_t = typename evaluate_ast<T>::type;
    }

    // ============================================================================
    // 2. MATH NAMESPACE: Algebraic Policies & Core Operations
    // ============================================================================
    namespace math {
        // Hard compiler boundary restricting input to mathematical integer rings (Z)
        template<typename T>
        concept IsIntegralOrMultiprecision = std::integral<meta::eval_t<T>> || requires {
            requires boost::multiprecision::number_category<meta::eval_t<T>>::value ==
                     boost::multiprecision::number_kind_integer;
        };

        template<IsIntegralOrMultiprecision T>
        auto compute_gcd(const T &a, const T &b) {
            using BaseType = meta::eval_t<T>;
            if constexpr (std::is_fundamental_v<BaseType>) {
                return std::gcd(a, b);
            } else {
                return boost::multiprecision::gcd(a, b);
            }
        }

        // Isolated Undefined Behavior (UB) policy for hardware overflow constraints
        template<IsIntegralOrMultiprecision T>
        meta::eval_t<T> safe_abs(T&& val) {
            using BaseType = meta::eval_t<T>;
            BaseType res(std::forward<T>(val));
            if constexpr (std::is_fundamental_v<BaseType>) {
                if constexpr (std::is_signed_v<BaseType>) {
                    if (res == std::numeric_limits<BaseType>::min()) {
                        return std::numeric_limits<BaseType>::max();
                    }
                    if (res < 0) return -res;
                }
            } else {
                if (res < 0) return -res;
            }
            return res;
        }

        template<IsIntegralOrMultiprecision T>
        meta::eval_t<T> power(meta::eval_t<T> base, uint32_t exp) {
            using BaseType = meta::eval_t<T>;
            BaseType res(1);
            while (exp > 0) {
                if (exp & 1) res *= base;
                exp >>= 1;
                if (exp == 0) break; // Asymptotic boundary protection
                base *= base;
            }
            return res;
        }

        template<IsIntegralOrMultiprecision T>
        std::pair<meta::eval_t<T>, meta::eval_t<T>> fib_pair(uint32_t n) {
            using BaseType = meta::eval_t<T>;
            if (n == 0) return {BaseType(0), BaseType(1)};

            auto p = fib_pair<BaseType>(n >> 1);
            BaseType c = p.first * (BaseType(2) * p.second - p.first);
            BaseType d = p.first * p.first + p.second * p.second;

            if (n & 1) {
                BaseType next = c + d;
                // RESOLVED: Sequence point violation via isolated materialization
                return {std::move(d), std::move(next)};
            } else {
                return {std::move(c), std::move(d)};
            }
        }
    }

    // ============================================================================
    // 3. PRIMES NAMESPACE: Sieve Mechanics
    // ============================================================================
    namespace primes {
        inline std::vector<uint32_t> generate_primes(size_t M, bool skip_two) {
            if (M == 0) return {};
            std::vector<uint32_t> prime_list;
            prime_list.reserve(M);

            if (!skip_two) prime_list.push_back(2);
            if (prime_list.size() == M) return prime_list;

            size_t target_count = skip_two ? M + 1 : M;
            size_t max_prime = 15;

            if (target_count >= 6) {
                double n = static_cast<double>(target_count);
                max_prime = static_cast<size_t>(n * (std::log(n) + std::log(std::log(n))));
            }
            max_prime = static_cast<size_t>(max_prime * 1.15) + 50;

            std::vector<bool> is_prime(max_prime + 1, true);
            is_prime[0] = is_prime[1] = false;

            for (size_t p = 2; p * p <= max_prime; ++p) {
                if (is_prime[p]) {
                    for (size_t i = p * p; i <= max_prime; i += p) {
                        is_prime[i] = false;
                    }
                }
            }

            for (size_t p = skip_two ? 3 : 2; p <= max_prime && prime_list.size() < M; ++p) {
                if (is_prime[p]) prime_list.push_back(static_cast<uint32_t>(p));
            }
            return prime_list;
        }
    }

    // ============================================================================
    // 4. CORE NAMESPACE: Engine Mechanisms & Public API
    // ============================================================================
    namespace core {

        template<math::IsIntegralOrMultiprecision T>
        meta::eval_t<T> extract_sm_regular_seed(T&& N_in, const std::vector<uint32_t>& Sm) {
            using BaseType = meta::eval_t<T>;
            BaseType N = math::safe_abs(std::forward<T>(N_in));

            if (N < 2) return BaseType(2); // Floor boundary protects against division by zero

            BaseType P(1);
            auto flush_chunk = [&N](BaseType& chunk_P) {
                BaseType G = math::compute_gcd<BaseType>(N, chunk_P);
                while (G > 1) {
                    N /= G;
                    if constexpr (std::is_fundamental_v<BaseType>) {
                        if (G > std::numeric_limits<BaseType>::max() / G) {
                            G = math::compute_gcd<BaseType>(N, G);
                        } else {
                            G = math::compute_gcd<BaseType>(N, G * G);
                        }
                    } else {
                        G = math::compute_gcd<BaseType>(N, G * G);
                    }
                }
                chunk_P = 1;
            };

            constexpr uint64_t MAX_CHUNK_LIMIT = std::numeric_limits<uint64_t>::max();

            for (uint32_t p : Sm) {
                if constexpr (std::is_fundamental_v<BaseType>) {
                    if (P > std::numeric_limits<BaseType>::max() / p) flush_chunk(P);
                } else {
                    if (P > MAX_CHUNK_LIMIT / p) flush_chunk(P);
                }
                P *= p;
            }

            if (P > 1) flush_chunk(P);
            return (N < 2) ? BaseType(2) : N;
        }

        template<math::IsIntegralOrMultiprecision T>
        std::vector<meta::eval_t<T>> generate_track1_cyclotomic(T&& entropy_seed, size_t M) {
            using BaseType = meta::eval_t<T>;
            if (M == 0) return {};

            std::vector<uint32_t> s_bound = primes::generate_primes(M, false);
            BaseType A_seed = extract_sm_regular_seed(std::forward<T>(entropy_seed), s_bound);

            std::vector<uint32_t> eval_primes = primes::generate_primes(M, true);
            std::vector<BaseType> result;
            result.reserve(M);

            BaseType A_minus_1 = A_seed - BaseType(1);

            for (uint32_t p_i : eval_primes) {
                BaseType A_pow_p = math::power<BaseType>(A_seed, p_i);
                BaseType d_i = (A_pow_p - BaseType(1)) / A_minus_1;
                result.push_back(std::move(d_i));
            }
            return result;
        }

        template<math::IsIntegralOrMultiprecision T>
        std::vector<meta::eval_t<T>> generate_track2_sds(size_t M) {
            using BaseType = meta::eval_t<T>;
            if (M == 0) return {};

            std::vector<uint32_t> eval_primes = primes::generate_primes(M, false);
            std::vector<BaseType> result;
            result.reserve(M);

            for (uint32_t p_i : eval_primes) {
                BaseType d_i = math::fib_pair<BaseType>(p_i).first;
                result.push_back(std::move(d_i));
            }
            return result;
        }
    }
}
#endif // COPRIME_ENGINE_HPP
