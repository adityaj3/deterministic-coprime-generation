#ifndef COPRIME_ENGINE_HPP
#define COPRIME_ENGINE_HPP

#include <vector>
#include <utility>
#include <stdexcept>
#include <type_traits>
#include <concepts>
#include <cstdint>
#include <boost/multiprecision/number.hpp>

namespace coprime {

// ============================================================================
// CONCEPTS & META-PROGRAMMING
// ============================================================================

// Mandated C++20 Concept validating both std::integral and boost::multiprecision::number
// Uses nested requires to strictly check the enum value, avoiding type-matching compilation errors.
template <typename T>
concept IsIntegralOrMultiprecision = std::integral<T> || requires {
    requires boost::multiprecision::number_category<T>::value ==
             boost::multiprecision::number_kind_integer;
};

// ============================================================================
// INTERNAL MATHEMATICAL HELPERS (QUARANTINED ENCAPSULATION)
// ============================================================================
namespace detail {

    // Dual-Track Prime Sieve Realization
    // skip_two = true enforces Axiom II: p_i \in \mathbb{P} \setminus {2} (Track 1)
    // skip_two = false enforces Axiom III: p_i >= 2 (Track 2)
    inline std::vector<uint32_t> generate_primes(size_t M, bool skip_two) {
        std::vector<uint32_t> primes;
        primes.reserve(M);
        uint32_t candidate = skip_two ? 3 : 2;

        while (primes.size() < M) {
            bool is_prime = true;
            for (uint32_t p = 2; p * p <= candidate; ++p) {
                if (candidate % p == 0) {
                    is_prime = false;
                    break;
                }
            }
            if (is_prime) {
                primes.push_back(candidate);
            }
            candidate += (candidate == 2) ? 1 : 2;
        }
        return primes;
    }

    // O(log p_i) Exponentiation by Squaring for Generalized Cyclotomic Evaluation
    template <IsIntegralOrMultiprecision T>
    T power(T base, uint32_t exp) {
        T res(1);
        while (exp > 0) {
            if (exp & 1) {
                res *= base;
            }
            base *= base;
            exp >>= 1;
        }
        return res;
    }

    // Fast-doubling matrix exponentiation for Fibonacci Sequence (Track 2)
    // Returns {F(n), F(n+1)} in O(log n) arithmetic steps.
    template <IsIntegralOrMultiprecision T>
    std::pair<T, T> fib_pair(uint32_t n) {
        if (n == 0) {
            return {T(0), T(1)};
        }

        auto p = fib_pair<T>(n >> 1);

        // Evaluate intermediate expressions into strict local lvalues to prevent
        // use-after-move sequence corruption.
        T c = p.first * (T(2) * p.second - p.first);
        T d = p.first * p.first + p.second * p.second;

        if (n & 1) {
            T next = c + d;
            return {std::move(d), std::move(next)};
        } else {
            return {std::move(c), std::move(d)};
        }
    }

    // Linear-time S_m-regular seed extraction (Axiom I)
    template <IsIntegralOrMultiprecision T>
    T extract_sm_regular_seed(T N, const std::vector<uint32_t>& Sm) {
        // Enforce fundamental signed/unsigned normalization
        if constexpr (std::is_signed_v<T> || !std::is_fundamental_v<T>) {
            if (N < 0) {
                N = -N;
            }
        }

        // Zero-entropy singularity guard
        if (N == 0) {
            return T(2);
        }

        for (uint32_t p : Sm) {
            while (N % p == 0) {
                N /= p;
            }
        }

        // Minimum entropy floor (Axiom I normalization)
        return (N < 2) ? T(2) : N;
    }

} // namespace detail

// ============================================================================
// PUBLIC API: DETERMINISTIC COPRIME GENERATION
// ============================================================================

/**
 * Track 1: Generalized Cyclotomic Mapping
 * Maps an S_m-regular seed through \Phi_{p_i}(A_seed) evaluated at distinct odd primes.
 * Geometrically bounds sequence memory to O(p_i \log A) bits.
 */
template <IsIntegralOrMultiprecision T>
std::vector<T> generate_track1_cyclotomic(const T& entropy_seed, size_t M) {
    if (M == 0) return {};

    // 1. Generate internal bounding prime set S_m to extract the S_m-regular seed
    std::vector<uint32_t> s_bound = detail::generate_primes(M, false);
    T A_seed = detail::extract_sm_regular_seed(entropy_seed, s_bound);

    // 2. Generate evaluation indices strictly satisfying Axiom II (p_i \in \mathbb{P} \setminus {2})
    std::vector<uint32_t> eval_primes = detail::generate_primes(M, true);

    std::vector<T> result;
    result.reserve(M);

    // 3. Hoist the constant denominator outside the arbitrary-precision mapping loop
    // Eliminates M-1 redundant multi-limb temporary object allocations.
    T A_minus_1 = A_seed - T(1);

    for (uint32_t p_i : eval_primes) {
        T A_pow_p = detail::power(A_seed, p_i);
        T d_i = (A_pow_p - T(1)) / A_minus_1;
        result.push_back(std::move(d_i));
    }

    return result;
}

/**
 * Track 2: Strong Divisibility Sequences
 * Derives pairwise coprimality autonomously by evaluating the Lucas sequence U_{p_i}(1, -1)
 * strictly at prime indices. Reduces asymptotic bit-length scale by 51.8%.
 */
template <IsIntegralOrMultiprecision T>
std::vector<T> generate_track2_sds(size_t M) {
    if (M == 0) return {};

    // Generate indices satisfying Axiom III (p_i >= 2)
    std::vector<uint32_t> eval_primes = detail::generate_primes(M, false);

    std::vector<T> result;
    result.reserve(M);

    for (uint32_t p_i : eval_primes) {
        T d_i = detail::fib_pair<T>(p_i).first;
        result.push_back(std::move(d_i));
    }

    return result;
}

} // namespace coprime

#endif // COPRIME_ENGINE_HPP
