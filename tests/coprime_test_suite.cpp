#include <catch2/catch_test_macros.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <execution>
#include <numeric>
#include <vector>
#include <cstdint>
#include <cmath>
#include <atomic>
#include <iostream>

#include <coprime_engine.hpp>

// ============================================================================
// PARALLEL VERIFICATION PRIMITIVE (STRICT NAMESPACE ISOLATION & OOM SAFETY)
// ============================================================================

namespace {
    // Escaped to internal linkage via anonymous namespace to prevent ODR violations
    template<typename T>
    bool verify_pairwise_coprimality_parallel(const std::vector<T> &vec) {
        if (vec.size() < 2) return true;

        std::vector<size_t> indices(vec.size());
        std::iota(indices.begin(), indices.end(), 0);

        // Lock-free atomic short-circuit flag to intercept OOM thread thrashing
        std::atomic<bool> memory_fault{false};

        bool is_coprime = std::transform_reduce(
            std::execution::par,
            indices.begin(),
            indices.end(),
            true,
            std::logical_and<bool>{},
            [&](size_t i) {
                // 1. ATOMIC SHORT-CIRCUIT: Abort immediately if sibling thread starved DRAM
                if (memory_fault.load(std::memory_order_relaxed)) {
                    return false;
                }

                // 2. EXCEPTION SAFETY: Intercept unhandled std::bad_alloc in thread pool
                try {
                    bool local_coprime = true;
                    for (size_t j = i + 1; j < vec.size(); ++j) {
                        if (coprime::math::compute_gcd<T>(vec[i], vec[j]) != 1) {
                            local_coprime = false;
                            break;
                        }
                    }
                    return local_coprime;
                } catch (const std::bad_alloc &) {
                    // 3. FAULT PROPAGATION: Signal early exit to all executing lanes
                    memory_fault.store(true, std::memory_order_relaxed);
                    return false;
                } catch (...) {
                    // Catch-all to prevent unhandled exceptions reaching std::terminate
                    memory_fault.store(true, std::memory_order_relaxed);
                    return false;
                }
            }
        );

        // If DRAM was exhausted, fail the test safely without catastrophic process termination
        if (memory_fault.load(std::memory_order_relaxed)) {
            std::cerr << "\n[!] SYSTEM EXCEPTION: std::bad_alloc caught during parallel verification.\n"
                    << "    DRAM bounds exceeded. Terminating verification gracefully.\n";
            return false;
        }

        return is_coprime;
    }
}

// ============================================================================
// TEST SUITE: BOUNDARY AXIOMS
// ============================================================================

TEST_CASE("Boundary Axioms: Zero-Entropy and Total Factor Absorption", "[track1][boundaries]") {
    uint64_t entropy_zero = 0;
    uint64_t entropy_one = 1;
    size_t M = 10;

    auto res_zero = coprime::core::generate_track1_cyclotomic(entropy_zero, M);
    auto res_one = coprime::core::generate_track1_cyclotomic(entropy_one, M);

    REQUIRE(res_zero.size() == M);
    REQUIRE(res_one.size() == M);

    REQUIRE(res_zero[0] == 7);
    REQUIRE(res_one[0] == 7);

    REQUIRE(verify_pairwise_coprimality_parallel(res_zero));
    REQUIRE(verify_pairwise_coprimality_parallel(res_one));
}

// ============================================================================
// TEST SUITE: DYNAMIC SCALING (FUNDAMENTAL & MULTIPRECISION)
// ============================================================================

TEST_CASE("Dynamic Scaling and Type Bounding: Fundamental and Multiprecision", "[scaling]") {
    std::vector<size_t> target_sizes = {5, 10, 50, 100};

    for (size_t M: target_sizes) {
        DYNAMIC_SECTION("Evaluating target set size M=" << M) {
            if (M <= 10) {
                uint64_t fund_seed = 84;
                auto t1_fund = coprime::core::generate_track1_cyclotomic(fund_seed, M);
                auto t2_fund = coprime::core::generate_track2_sds<uint64_t>(M);

                REQUIRE(verify_pairwise_coprimality_parallel(t1_fund));
                REQUIRE(verify_pairwise_coprimality_parallel(t2_fund));
            }

            boost::multiprecision::cpp_int mp_seed("18446744073709551615");

            auto t1_mp = coprime::core::generate_track1_cyclotomic(mp_seed, M);
            auto t2_mp = coprime::core::generate_track2_sds<boost::multiprecision::cpp_int>(M);

            REQUIRE(t1_mp.size() == M);
            REQUIRE(t2_mp.size() == M);

            REQUIRE(verify_pairwise_coprimality_parallel(t1_mp));
            REQUIRE(verify_pairwise_coprimality_parallel(t2_mp));
        }
    }
}

// ============================================================================
// TEST SUITE: MULTIPRECISION STRESS TEST
// ============================================================================

TEST_CASE("Multiprecision Stress Test: OOM-Proof Chunked Primorial Flush", "[track1][stress]") {
    boost::multiprecision::cpp_int massive_seed(
        "340282366920938463463374607431768211456000000000000000000000000000000000000");
    size_t M = 500;

    auto t1_res = coprime::core::generate_track1_cyclotomic(massive_seed, M);

    REQUIRE(t1_res.size() == M);
    REQUIRE(verify_pairwise_coprimality_parallel(t1_res));
}

// ============================================================================
// TEST SUITE: CONCURRENCY EXTREME STRESS TEST
// ============================================================================

TEST_CASE("Concurrency Hard Rule: O(N^2) Verification at Extreme Scale", "[track2][concurrency]") {
    size_t M = 1000;
    auto t2_res = coprime::core::generate_track2_sds<boost::multiprecision::cpp_int>(M);

    REQUIRE(t2_res.size() == M);
    REQUIRE(verify_pairwise_coprimality_parallel(t2_res));
}

// ============================================================================
// TEST SUITE: EXACT RATIONAL AUDIT
// ============================================================================

TEST_CASE("Exact Empirical Audit: Theoretical Spatial Memory Contraction Limits", "[track2][audit]") {
    size_t M = 200;
    auto t2_res = coprime::core::generate_track2_sds<boost::multiprecision::cpp_int>(M);

    size_t t2_max_bits = 0;
    for (const auto &val: t2_res) {
        size_t bits = (val == 0 ? 0 : static_cast<size_t>(boost::multiprecision::msb(val) + 1));
        if (bits > t2_max_bits) t2_max_bits = bits;
    }

    double primorial_bits = 0.0;
    auto primes = coprime::primes::generate_primes(M, false);
    for (uint64_t p: primes) {
        primorial_bits += std::log2(static_cast<double>(p));
    }

    double empirical_contraction = 1.0 - (static_cast<double>(t2_max_bits) / primorial_bits);
    constexpr double THEORETICAL_LIMIT = 0.518789; // 1 - ln(phi)

    // Tolerance buffer accounts for discrete float approximation margins at M=200
    REQUIRE(std::abs(empirical_contraction - THEORETICAL_LIMIT) < 0.05);
}
