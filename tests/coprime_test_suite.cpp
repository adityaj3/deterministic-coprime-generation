// ============================================================================
// STRICT C++20 COMPILE DIRECTIVES
// Requires linking against Catch2 (v3) and TBB (for std::execution::par)
// CMake: target_link_libraries(test_suite Catch2::Catch2WithMain)
// ============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/integer/common_factor.hpp>
#include <vector>
#include <numeric>
#include <execution>
#include <atomic>
#include <cmath>
#include <iostream>
#include <string>

// Include the previously generated Deliverable 1 engine
#include "coprime_engine.hpp"

using namespace boost::multiprecision;
using BigInt = cpp_int;

// ============================================================================
// VERIFICATION HELPER FUNCTIONS
// ============================================================================

// Validates strictly pairwise coprimality: gcd(d_i, d_j) == 1 for all i != j
bool verify_pairwise_coprimality(const std::vector<BigInt>& seq) {
    size_t M = seq.size();
    if (M < 2) return true;

    for (size_t i = 0; i < M; ++i) {
        for (size_t j = i + 1; j < M; ++j) {
            if (boost::multiprecision::gcd(seq[i], seq[j]) != 1) {
                return false;
            }
        }
    }
    return true;
}

// ============================================================================
// TEST CASE 1: TRACK 1 (CYCLOTOMIC) - BOUNDED DIMENSIONAL SCALING
// ============================================================================

TEST_CASE("Track 1: Bounded Dimensional Scaling (Multi-Limb Transition)", "[track1][coprimality]") {
    // M=5 tests single-limb boundary. M=30 bounds the Lehmer GCD to ~117 limbs
    // to prevent O(B^2) multi-limb quadratic stalling.
    const std::vector<size_t> t1_sizes = {5, 10, 20, 30};

    // 20-digit string ensures multi-limb carry propagation without stalling
    BigInt base_entropy("98765432101234567890");

    for (size_t M : t1_sizes) {
        // DYNAMIC_SECTION prevents Catch2 AST re-entry overhead.
        // CHECK ensures execution continues across all parameters upon failure.
        DYNAMIC_SECTION("Evaluating Track 1 for M = " << M) {
            std::vector<BigInt> t1_seq = coprime::generate_track1_cyclotomic<BigInt>(base_entropy, M);

            CHECK(t1_seq.size() == M);
            CHECK(verify_pairwise_coprimality(t1_seq) == true);
        }
    }
}

// ============================================================================
// TEST CASE 2: TRACK 2 (SDS) - UNBOUNDED SCALING & ASYMPTOTIC AUDIT
// ============================================================================

TEST_CASE("Track 2: Unbounded Dimensional Scaling & Asymptotic Audit", "[track2][scaling][asymptotic]") {
    const std::vector<size_t> t2_sizes = {10, 50, 100, 250, 500};

    // Theoretical complexity constants
    const double log2_phi = 0.6942419136306174;
    const double log2_5_half = 1.1609640474436813;
    const double log2_e = 1.4426950408889634;

    for (size_t M : t2_sizes) {
        DYNAMIC_SECTION("Evaluating Track 2 for M = " << M) {
            std::vector<BigInt> t2_seq = coprime::generate_track2_sds<BigInt>(M);

            CHECK(t2_seq.size() == M);

            // Phase 1: Pairwise GCD Verification
            CHECK(verify_pairwise_coprimality(t2_seq) == true);

            // Phase 2: Asymptotic Bit-Length Empirical Validation
            std::vector<uint32_t> eval_primes = coprime::detail::generate_primes(M, false);
            uint32_t p_M = eval_primes.back();
            BigInt final_element = t2_seq.back();

            // Extract exact bit-length using boost built-in MSB
            unsigned actual_bit_length = msb(final_element) + 1;

            // Theoretical Bounds Calculation
            double expected_fib_bits = (p_M * log2_phi) - log2_5_half;
            double expected_prim_bits = p_M * log2_e;

            // Assert exact geometric length bound (Tolerance +/- 2 bits due to Binet floor)
            CHECK_THAT(static_cast<double>(actual_bit_length),
                       Catch::Matchers::WithinAbs(expected_fib_bits, 2.0));

            // Calculate actual and dynamic theoretical space reduction ratio
            double actual_reduction_ratio = 1.0 - (static_cast<double>(actual_bit_length) / expected_prim_bits);
            double expected_reduction_ratio = 1.0 - (expected_fib_bits / expected_prim_bits);

            // Validate that actual space reduction strictly tracks theoretical prediction (Tolerance +/- 0.005)
            CHECK_THAT(actual_reduction_ratio,
                       Catch::Matchers::WithinAbs(expected_reduction_ratio, 0.005));
        }
    }
}

// ============================================================================
// TEST CASE 3: EXTREME OPT-IN PARALLEL VERIFICATION (M=1000)
// ============================================================================

TEST_CASE("Track 2: Extreme Opt-In Parallel Verification (M=1000)", "[.][parallel][extreme]") {
    size_t M = 1000;
    std::vector<BigInt> seq = coprime::generate_track2_sds<BigInt>(M);
    REQUIRE(seq.size() == M);

    // Pre-compute index pairs to allow flat parallel execution map
    std::vector<std::pair<size_t, size_t>> indices;
    indices.reserve((M * (M - 1)) / 2);
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = i + 1; j < M; ++j) {
            indices.emplace_back(i, j);
        }
    }

    std::atomic<bool> all_pairwise_coprime{true};

    // Parallelize the 499,500 GCD checks to prevent test runner timeouts
    std::for_each(std::execution::par, indices.begin(), indices.end(),
        [&seq, &all_pairwise_coprime](const std::pair<size_t, size_t>& p) {
            if (boost::multiprecision::gcd(seq[p.first], seq[p.second]) != 1) {
                all_pairwise_coprime.store(false, std::memory_order_relaxed);
            }
        }
    );

    CHECK(all_pairwise_coprime.load() == true);
}

// ============================================================================
// TEST CASE 4: BOUNDARY STATE SWEEP (SINGULARITY MITIGATION)
// ============================================================================

TEST_CASE("Track 1: Boundary State & Singularity Sweep", "[boundary][axioms]") {
    size_t M = 20;

    // Utilize isolated scope blocks instead of standard SECTIONs to guarantee
    // single-pass sequential execution without invoking AST teardowns.

    {
        // State 1: Zero-Entropy Input (N=0) implies v_p(0)=infty
        // Axiom I must sanitize N=0 to A_seed=2 to prevent trivial array of 1s
        std::vector<BigInt> seq = coprime::generate_track1_cyclotomic<BigInt>(BigInt(0), M);
        CHECK(verify_pairwise_coprimality(seq) == true);
        CHECK(seq[0] > 1); // Explicitly confirm sequence is not trivial 1s
    }

    {
        // State 2: Identity Input (N=1)
        // Axiom I must sanitize N=1 to A_seed=2
        std::vector<BigInt> seq = coprime::generate_track1_cyclotomic<BigInt>(BigInt(1), M);
        CHECK(verify_pairwise_coprimality(seq) == true);
        CHECK(seq[0] > 1);
    }

    {
        // State 3: Power-of-Two Input (N=2^1024)
        BigInt pow2 = cpp_int(1) << 1024;
        std::vector<BigInt> seq = coprime::generate_track1_cyclotomic<BigInt>(pow2, M);
        CHECK(verify_pairwise_coprimality(seq) == true);
    }

    {
        // State 4: Total Factor Absorption (N = P_m#)
        // Construct primorial P_m#
        std::vector<uint32_t> primes = coprime::detail::generate_primes(M, false);
        BigInt primorial = 1;
        for (uint32_t p : primes) {
            primorial *= p;
        }

        // When N = P_m#, S_m-regular stripping yields exactly 1.
        // Axiom I sanitizes 1 -> 2.
        std::vector<BigInt> seq = coprime::generate_track1_cyclotomic<BigInt>(primorial, M);
        CHECK(verify_pairwise_coprimality(seq) == true);
        CHECK(seq[0] > 1);
    }
}
