#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <type_traits>
#include <boost/multiprecision/cpp_int.hpp>

#include <coprime_engine.hpp>

using boost::multiprecision::cpp_int;
using boost::multiprecision::msb;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;

template<typename T>
size_t extract_container_size(const T &obj) {
    if constexpr (requires { obj.size(); }) {
        return obj.size();
    } else if constexpr (requires { obj.first.size(); }) {
        return obj.first.size();
    } else {
        return 0;
    }
}

// ============================================================================
// REMEDIATION: EXACT TRIANGULAR SPATIAL MASS SUMMATION (NO RECTANGULAR BLOAT)
// ============================================================================
inline size_t extract_total_bits(const std::vector<cpp_int> &sequence) {
    size_t total_bits = 0;
    for (const auto &val: sequence) {
        // Bitwise Zero Guard: Protect against undefined behavior on msb(0)
        total_bits += (val == 0 ? 0 : static_cast<size_t>(msb(val) + 1));
    }
    return total_bits;
}

std::pair<std::vector<cpp_int>, uint64_t> naive_rejection_sampler(size_t M) {
    std::vector<cpp_int> result;
    result.reserve(M);

    cpp_int candidate = 2;
    uint64_t gcd_calls = 0;

    while (result.size() < M) {
        bool coprime = true;
        for (const auto &existing: result) {
            gcd_calls++;
            if (boost::multiprecision::gcd(candidate, existing) != 1) {
                coprime = false;
                break;
            }
        }
        if (coprime) {
            result.push_back(candidate);
        }
        candidate++;
    }

    return {std::move(result), gcd_calls};
}

void global_heap_allocation_pre_warm(const cpp_int &seed) {
    std::cout << "[*] Executing pre-warm phase (M=500)...\n";
    for (size_t i = 0; i < 50; ++i) {
        auto t1 = coprime::core::generate_track1_cyclotomic(seed, 500);
        auto t2 = coprime::core::generate_track2_sds<cpp_int>(500);

        volatile size_t s1 = t1.size();
        volatile size_t s2 = t2.size();
        (void) s1;
        (void) s2;
    }
    std::cout << "[*] Pre-warm complete. Allocators and cache stabilized.\n\n";
}

template<typename Func>
std::pair<double, decltype(std::declval<Func>()())> measure_median_latency_and_get_result(
    size_t iterations, Func &&func) {
    std::vector<double> times_us;
    times_us.reserve(iterations);

    using ReturnType = decltype(std::declval<Func>()());
    ReturnType sample_result;

    for (size_t i = 0; i < iterations; ++i) {
        auto start = high_resolution_clock::now();
        auto res = func();
        auto end = high_resolution_clock::now();

        volatile size_t dummy = extract_container_size(res);
        (void) dummy;

        double us = static_cast<double>(duration_cast<nanoseconds>(end - start).count()) / 1000.0;
        times_us.push_back(us);

        if (i == iterations - 1) {
            sample_result = std::move(res);
        }
    }

    std::sort(times_us.begin(), times_us.end());
    double median_us = times_us[iterations / 2];

    return {median_us, std::move(sample_result)};
}

int main() {
    const std::vector<size_t> scales = {10, 50, 100, 200, 500, 1000, 2000, 5000, 10000};
    const cpp_int entropy_seed("340282366920938463463374607431768211456");

    global_heap_allocation_pre_warm(entropy_seed);

    std::ofstream out("telemetry.json");
    out << "[\n";

    for (size_t i = 0; i < scales.size(); ++i) {
        const size_t M = scales[i];
        const size_t N_runs = (M <= 1000) ? 100 : 10;

        auto [t1_lat, t1_res] = measure_median_latency_and_get_result(N_runs, [&]() {
            return coprime::core::generate_track1_cyclotomic(entropy_seed, M);
        });

        auto [t2_lat, t2_res] = measure_median_latency_and_get_result(N_runs, [&]() {
            return coprime::core::generate_track2_sds<cpp_int>(M);
        });

        auto [naive_lat, naive_pair] = measure_median_latency_and_get_result(N_runs, [&]() {
            return naive_rejection_sampler(M);
        });

        uint64_t naive_gcds = naive_pair.second;

        double primorial_bits_approx = 0.0;
        auto primes = coprime::primes::generate_primes(M, false);
        for (uint64_t p: primes) {
            primorial_bits_approx += std::log2(static_cast<double>(p));
        }

        // REMEDIATION: Purged scalar max-bits. Exporting exact evaluated spatial volume.
        out << "  {\n"
                << "    \"M\": " << M << ",\n"
                << "    \"T1_latency_us\": " << t1_lat << ",\n"
                << "    \"T2_latency_us\": " << t2_lat << ",\n"
                << "    \"Naive_latency_us\": " << naive_lat << ",\n"
                << "    \"T1_total_bits\": " << extract_total_bits(t1_res) << ",\n"
                << "    \"T2_total_bits\": " << extract_total_bits(t2_res) << ",\n"
                << "    \"Naive_total_bits\": " << extract_total_bits(naive_pair.first) << ",\n"
                << "    \"Primorial_bits\": " << static_cast<size_t>(std::ceil(primorial_bits_approx)) << ",\n"
                << "    \"Naive_GCDs\": " << naive_gcds << "\n"
                << "  }" << (i < scales.size() - 1 ? "," : "") << "\n";

        std::cout << "[*] Inspector completed scale M = " << M << " (N = " << N_runs << " runs)\n";
    }

    out << "]\n";
    out.close();

    std::cout << "[+] Telemetry extraction complete. Exported strictly bounded spatial data to 'telemetry.json'.\n";
    return 0;
}
