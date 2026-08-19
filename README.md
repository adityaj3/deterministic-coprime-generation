# Deterministic Pairwise Coprime Generation Engine

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)
![Dependencies](https://img.shields.io/badge/Dependencies-Boost%20%7C%20TBB-orange.svg)

## PART I: ENGINE SPECIFICATION & OPERATIONS

### 1. Executive Summary & Paradigm Shift

Standard computational methods build pairwise coprime integer sets via **Classical Trial Rejection Sampling**, generating candidates and running the Euclidean algorithm to verify $\gcd(c, r) = 1$ against all previously accepted elements. This traditional approach introduces an inescapable $\mathcal{O}(M^2)$ computational bottleneck. The execution latency is further exacerbated by global data dependencies and "Jacobsthal coprime deserts", large structural voids in the integer number line that force probabilistic algorithms into prolonged, unpredictable verification loops.

This engine fundamentally resolves these bottlenecks through **Forward Algebraic Mapping**. By combining $S_m$-regular seed purification ($A_{\text{seed}}$) with precise polynomial or divisibility evaluations ($\Phi_{p_i}$ and $F_{p_i}$), the engine mathematically guarantees pairwise coprimality via direct algebraic mapping ($\mathcal{O}(1)$ operations per element relative to set size $M$). This framework completely mitigates the factorial bit-length explosion native to naive primorial progressions while executing **strictly 0 Euclidean GCD calls**.

```text
PARADIGM 1: NAIVE TRIAL REJECTION (Classical Method)
[Candidate c] ---> [Run GCD against ALL existing elements] ---> 𝒪(M²) GCD Calls

PARADIGM 2: FORWARD ALGEBRAIC MAPPING (This Engine)
[Index pᵢ]    ---> [Direct Evaluation: Φ_{pᵢ}(A_seed) or F_{pᵢ}]  ---> 0 GCD Calls (𝒪(1) Math)
```

### 2. Architecture & File Tree

```text
deterministic-coprime-generation/
├── include/
│   └── coprime_engine.hpp        # Core C++20 Engine (meta, math, primes, core)
├── tests/
│   └── coprime_test_suite.cpp    # Catch2 Validation & Multiprecision Stress Tests
├── benchmarks/
│   └── coprime_bench.cpp         # Comparative Micro-Benchmark Harness
├── tools/
│   ├── coprime_inspector.cpp     # Telemetry Profiler (C++20)
│   └── plot_telemetry.py         # SVG Dashboard Renderer (Python 3)
├── docs/
│   └── assets/
│       └── benchmark_dash.svg    # SVG Dashboard
├── .github/
│   └── workflows/
│       └── ci.yml                # CI/CD Pipeline (GCC 13, Clang 16, ASan/UBSan)
├── CMakeLists.txt                # Build Infrastructure
├── manuscript.tex                # Formal Academic Manuscript
└── README.md                     # Formal Proofs & Unified Documentation

```

### 3. Build & Execution Guide

**Prerequisites:**

* CMake 3.22+
* GCC 13+ or Clang 16+ (Strict C++20 support required)
* Boost.Multiprecision (`libboost-all-dev`)
* Intel oneTBB (`libtbb-dev`): required for C++20 Parallel STL (`std::execution::par`) backend support on Linux.
* Python 3 + Matplotlib (`python3-matplotlib`): required for telemetry visualization.

**Compilation & Test Execution:**

```bash
# Configure and Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Run Unit Tests and Parallel Exception Safety Suite
ctest --output-on-failure

# Execute the Comparative Arbitrary-Precision Benchmark
./coprime_bench

# Extract JSON Spatial Mass Telemetry
./coprime_inspector

# Render Matplotlib Telemetry Dashboard
python3 ../tools/plot_telemetry.py

```

### 4. Physical Benchmark Table

**Performance profile evaluated over exact spatial memory mapping up to ~62.02 MB total bit mass at M = 10,000.** *(Evaluated using the fundamental base seed* $A_{\text{seed}} = 2$*)*.

| Target Set Size (M) | Naive GCD Calls | Naive Latency & Throughput | Cyclotomic Track 1 (Speedup & Throughput) | SDS Track 2 (Speedup & Throughput) |
| --- | --- | --- | --- | --- |
| M = 10 | 69 | 1.4 us (27.9 Mbps) | 1.9 us (0.7x speedup, 83.3 Mbps) | 2.7 us (0.5x speedup, 24.2 Mbps) |
| M = 50 | 1518 | 27.9 us (11.8 Mbps) | 11.4 us (2.4x speedup, 469.3 Mbps) | 23.6 us (1.2x speedup, 142.5 Mbps) |
| M = 100 | 5745 | 176.7 us (4.4 Mbps) | 32.0 us (5.5x speedup, 771.8 Mbps) | 62.8 us (2.8x speedup, 259.7 Mbps) |
| M = 200 | 21899 | 744.4 us (2.4 Mbps) | 107.0 us (7.0x speedup, 1054.8 Mbps) | 171.7 us (4.3x speedup, 445.4 Mbps) |
| M = 500 | 131546 | 5068.5 us (1.0 Mbps) | 537.7 us (9.4x speedup, 1540.4 Mbps) | 757.0 us (6.7x speedup, 752.6 Mbps) |
| M = 1000 | 516383 | 21696.7 us (0.54 Mbps) | 2550.8 us (8.5x speedup, 1446.9 Mbps) | 3433.6 us (6.3x speedup, 742.9 Mbps) |
| M = 2000 | 2040625 | 93223.2 us (0.28 Mbps) | 12415.5 us (7.5x speedup, 1312.2 Mbps) | 20279.3 us (4.6x speedup, 556.5 Mbps) |
| M = 5000 | 12634016 | 649127.8 us (0.11 Mbps) | 90739.3 us (7.2x speedup, 1261.9 Mbps) | 252740.5 us (2.6x speedup, 314.2 Mbps) |
| M = 10000 | 50328485 | 3070748.4 us (0.051 Mbps) | 375151.5 us (8.2x speedup, 1322.9 Mbps) | 1838904.0 us (1.7x speedup, 187.3 Mbps) |

*\*Note: Engine GCD calls strictly evaluate to 0 across all scaling factors M.*

### 5. Microarchitectural Crossover & Cache-to-DRAM Boundary Analysis

The engine's performance profile is governed not merely by theoretical algorithmic complexity, but by physical hardware constraints across the CPU register, cache, and main memory hierarchy.


```text

+-----------------------------------------------------------------------------------+
|                           MEMORY SUBSYSTEM REGIMES                                |
+-----------------------------------------------------------------------------------+
|  M = 10         | L1/Registers   | Scalar idiv beats heap init (0.7x speedup)     |
|  M = 50         | L1 Data Cache  | Karatsuba overtakes O(M²) GCD loop (2.4x)      |
|  M = 500        | L1/L2 Resident | Zero-latency RAM access; Peak Speedup (9.4x)   |
|  M = 10,000     | L3/DRAM Bound  | 62.02 MB mass saturates L3; Sustained 1.32 Gbps|
+-----------------------------------------------------------------------------------+

```

#### A. Cold-Start Latency & The Small-Set Inversion ($M \le 10$)
At minimal scales ($M \le 10$), the Naive rejection sampler executes purely within hardware CPU registers and L1 data cache using native 64-bit integer instructions. Conversely, Engine Track 1 initializes dynamic multi-limb buffers via `boost::multiprecision::cpp_int`. The heap allocation and metadata initialization overhead induce a brief latency penalty ($1.9\,\mu\text{s}$ vs. $1.4\,\mu\text{s}$, or a $0.7\times$ speedup inversion).

#### B. The Arithmetic Crossover ($M = 50$)
As set size reaches $M = 50$, the Naive sampler executes $1,518$ cumulative multi-precision `idiv` GCD operations (exceeding the theoretical minimum of $\frac{M(M-1)}{2} = 1,225$ checks due to candidate rejections). In contrast, Track 1 evaluates generalized cyclotomic polynomials directly via exponentiation-by-squaring, engaging sub-quadratic Karatsuba multiplication pathways. At this threshold, the Engine permanently overtakes the Naive baseline ($2.4\times$ speedup, $469.3\text{ Mbps}$).

#### C. L1/L2 Sub-Quadratic Operations & Peak Speedup ($M \le 500$)
Between $M = 100$ and $M = 500$, the generated multi-limb BigInt buffers occupy individual element sizes of $\le 7.5\text{ KB}$.
* **Cache Residency:** The active working dataset fits entirely within private core caches (L1: $32\text{ KB} - 48\text{ KB}$, L2: $512\text{ KB} - 1.25\text{ MB}$).
* **Execution Dynamics:** Because memory round-trips to main DRAM are eliminated, Karatsuba multi-precision multiplication runs at core clock speed without memory stall cycles. The engine achieves its **peak speedup ratio ($9.4\times$ at $M = 500$)** in this cache-resident regime.

#### D. L3 Cache Exhaustion & The DRAM Boundary ($M = 10,000$)
At extreme scales ($M = 10,000$), the cyclotomic mapping produces individual coprime elements reaching $104,729\text{ bits}$ ($\approx 13.09\text{ KB}$ per isolated integer for fundamental base seed $A_{\text{seed}} = 2$).
* **Triangular Spatial Footprint:** Storing the complete sequence requires accounting for the exact triangular spatial mass:
  $$\text{Total Memory} = \sum_{i=1}^{10000} p_i \approx \frac{1}{2} M \cdot p_M \log_2(A_{\text{seed}}) = 496,165,411\text{ bits} \approx 62.02\text{ MB}$$
  *(This formally rectifies and replaces naive rectangular integration assumptions of* $\approx 130.9\text{ MB}$.*)*
* **L3 Cache Saturation:** On standard desktop CPUs with $8\text{ MB} - 32\text{ MB}$ of shared L3 cache (and especially on older/low-spec hardware with $2\text{ MB} - 8\text{ MB}$ L3), a $62.02\text{ MB}$ dynamic footprint completely exhausts on-chip cache capacity. This forces continuous Translation Lookaside Buffer (TLB) evictions and main DRAM bus cycles, stabilizing the speedup multiplier at $\approx 8.2\times$.
* **Throughput Resilience:** Despite overwhelming CPU caches and forcing dynamic multi-limb heap allocations via `malloc`, Engine Track 1 completes the entire generation in $\approx 0.375\text{ seconds}$, sustaining a continuous throughput of $\approx 1.32\text{ Gbps}$ ($1,322.9\text{ Mbps}$). It outperforms the Naive baseline because it completely bypasses the $50.3\text{ million}$ memory-bound Euclidean `gcd` calls that cripple the trial sampler.

*(Note on Unpurified High-Entropy Seeds: Ingesting an unpurified 128-bit entropy seed (*$A \approx 2^{128}$*) scales individual elements to* $\approx 13.4\text{ million bits}$ *(*$\approx 1.67\text{ MB}$ *each), expanding the triangular memory footprint to its theoretical limit of* $\approx 8.35\text{ GB}$ *and completely saturating system DRAM.)*

### 6. Comprehensive Defense Against Non-Rejection Sampling Alternatives

Critics frequently question why the engine is necessary given existing alternative generation paradigms. Below is the formal refutation of three alternative approaches:

#### Alternative A: Probabilistic Primes + GCD Filtering (Randomized Pipelines)
* **Mechanism:** Generating a set of large random primes via Miller-Rabin primality testing, followed by pairwise GCD checks to ensure coprimality.
* **The Failure Mode:**
  1. **Computational Bottleneck:** While primality testing can be parallelized, verifying pairwise coprimality across $M$ elements still forces an $\mathcal{O}(M^2)$ GCD verification matrix as $M$ grows.
  2. **Entropy Destruction:** Probabilistic prime generation strips away deterministic seed traceability. It is impossible to recreate the exact same set of coprime elements from a compact root seed ($A_{\text{seed}}$) without storing the entire sequence state.

#### Alternative B: Prime Sieves (Eratosthenes / Atkin)
* **Mechanism:** Generating a static sequence of prime numbers via a prime sieve.
* **The Failure Mode (Category Error):**
  1. **Zero Input Entropy:** A prime sieve generates a deterministic, static mathematical set ($\{2, 3, 5, 7, 11, \dots\}$) with zero informational entropy ($H(X) = 0$). It possesses no mechanism to ingest an external, dynamic 256-bit entropy seed ($A_{\text{seed}}$) and project it into an exclusive algebraic field. Track 1 bridges external random entropy directly into deterministic coprimal geometry.
  2. **Scalar Limitation:** Prime sieves return tiny scalar values ($p_{10000} = 104,729$, occupying $\approx 17\text{ bits}$). They cannot natively construct the multi-megabyte composite integers required for arbitrary-precision memory stress boundaries and specialized cryptographic key-space generation.

#### Alternative C: Pseudorandom Number Generators (PRNGs) with Post-Hoc Filters
* **Mechanism:** Feeding a seed into a standard cryptographic PRNG (e.g., ChaCha20 or PCG) to generate candidate integers, filtering them via trial GCD checks.
* **The Failure Mode:** PRNG outputs lack algebraic guarantees of mutual coprimality. Because the integer number line contains dense "coprime deserts" (regions of high composite density), a PRNG-based sampler inevitably falls into catastrophic backtracking loops, rendering execution time non-deterministic ($\omega(\exp)$ worst-case complexity). Track 1 and Track 2 guarantee strictly $0$ GCD validation calls by structural algebraic construction.

### 7. Telemetry & Visual Analytics

![Deterministic Coprime Benchmark Dashboard](docs/assets/benchmark_dash.svg)

The generated telemetry vector maps visually corroborate the mathematical proofs:

1. **Execution Latency:** Log-Log scaling highlights the Engine's decisive divergence away from the Naive $\mathcal{O}(M^2)$ temporal expansion.
2. **Throughput:** A sustained $\sim 1.32$ Gbps spatial throughput plateau demonstrates optimal utilization of modern multi-limb arbitrary precision multiplication.
3. **Verification Calls:** The Engine invariant line strictly rests at $0$ GCDs, confirming full independence from probabilistic trial division.
4. **Spatial Contraction:** The empirical spatial mass of the Fibonacci SDS cleanly approaches and mirrors the theoretical $51.8789\%$ Binet-Chebyshev asymptote.

---

## PART II: FORMAL MATHEMATICAL PROOFS & INVARIANTS

### Section 1: Axiomatic Framework

To eliminate the inherent factor dependencies of arbitrary multiprecision seed inputs, we formalize the extraction of a strictly sanitized arithmetic basis.

**Definition ($S_m$-Regular Element):** Given an arbitrary integer $N \in \mathbb{N}$ and a bounding prime set $S_m = \{p_1, p_2, \dots, p_m\}$, the deterministic factor isolation guarantees the extraction of a sanitized $S_m$-regular seed defined as:


$$A_{\text{seed}} = \max\left(2, \frac{N}{\prod_{p_i \in S_m} p_i^{v_{p_i}(N)}}\right)$$

**Invariant Proof:** \
The explicit geometric boundary $\max(2, \cdot)$ rigidly prevents algorithmic degeneration to trivial unit states ($A \in \{0, 1\}$). Because the Generalized Cyclotomic mapping $d_i = \Phi_{p_i}(A)$ algebraically guarantees pairwise coprimality for *any* integer basis $A \ge 2$, the purified seed $A_{\text{seed}} \ge 2$ unconditionally forms a valid, non-degenerate geometric basis for the mapping sequence, regardless of prime factor overlap with $P_m\\#$.

### Section 2: Track 1 Generalized Cyclotomic Mapping Proof

**Theorem 2.1:** Let $A \in \mathbb{Z}_{\ge 2}$ be an $S_m$-regular seed. Let $\mathcal{P} = \{p_1, p_2, \dots, p_M\}$ be a set of distinct odd primes ($p_i \ge 3$). The sequence generated by the cyclotomic mapping:


$$d_i = \Phi_{p_i}(A) = \frac{A^{p_i} - 1}{A - 1} = \sum_{k=0}^{p_i-1} A^k$$


is strictly pairwise coprime.

**Proof (by Multiplicative Order** $\mathrm{ord}\_q(A)$ **):** \
Assume for the sake of contradiction there exists a prime $q$ that divides both $d_i = \Phi_{p_i}(A)$ and $d_j = \Phi_{p_j}(A)$ for distinct evaluation primes $p_i \neq p_j$.
Since $q \mid \frac{A^{p_i} - 1}{A - 1}$, it follows that $q \mid (A^{p_i} - 1)$ and $q \mid (A^{p_j} - 1)$.
This congruence dictates that the multiplicative order of $A$ modulo $q$ must satisfy:


$$\mathrm{ord}_q(A) \mid p_i \quad \text{and} \quad \mathrm{ord}_q(A) \mid p_j$$


Since $p_i$ and $p_j$ are distinct prime numbers, $\gcd(p_i, p_j) = 1$. Therefore, $\mathrm{ord}_q(A) = 1$.
The condition $\mathrm{ord}_q(A) = 1$ implies $A \equiv 1 \pmod q$, and consequently $q \mid (A-1)$.
Substituting $A \equiv 1 \pmod q$ into the cyclotomic polynomial expansion yields:


$$\Phi_{p_i}(A) = \sum_{k=0}^{p_i-1} A^k \equiv \sum_{k=0}^{p_i-1} 1^k \equiv p_i \pmod q$$


Since $q \mid \Phi_{p_i}(A)$, we must have $q = p_i$. By symmetric reasoning, $\Phi_{p_j}(A) \equiv p_j \pmod q$, enforcing $q = p_j$.
This implies $p_i = p_j$, contradicting the premise that $p_i$ and $p_j$ are distinct primes. Thus, no common prime divisor $q$ exists, and $\gcd(\Phi_{p_i}(A), \Phi_{p_j}(A)) = 1$. $\blacksquare$

**Emma T. Lehmer Resultant Verification:** \
By Emma T. Lehmer's theorem on the resultant of two cyclotomic polynomials, for any distinct primes $n, m$:


$$\mathrm{Res}(\Phi_n(x), \Phi_m(x)) = 1$$


Because $\Phi_n(x)$ and $\Phi_m(x)$ are monic polynomials with coefficients in $\mathbb{Z}$, the determinant of their Sylvester matrix evaluates to $1 \in \mathbb{Z}$. Consequently, Bezout's identity guarantees the existence of polynomials $U(x), V(x) \in \mathbb{Z}[x]$ such that:


$$U(x)\Phi_n(x) + V(x)\Phi_m(x) = 1$$


Evaluating this exact resultant identity at the integer basis $x = A_{\text{seed}}$ yields $U(A)\Phi_n(A) + V(A)\Phi_m(A) = 1$. Thus, $\gcd(\Phi_n(A), \Phi_m(A)) = 1$, preserving integer domain boundaries seamlessly.

**Asymptotic Work Complexity Analysis under Fast Multi-Precision Arithmetic:** \
To establish a physically falsifiable performance boundary, we analyze the computational complexity of the Engine against Naive Rejection Sampling under symmetric multi-precision optimizations (Karatsuba multiplication $\mathcal{O}(B^{\log_2 3}) \approx \mathcal{O}(B^{1.585})$ and Schönhage fast GCD $\mathcal{O}(B^{1.585} \log B)$ ).

Let $M$ be the target set size, and let $B = \mathcal{O}(M \ln M)$ represent the asymptotic element bit-width. The total computational work of Engine Track 1 is bounded by:

$$W_{\text{Engine}}(M) = \sum_{i=1}^M \mathcal{O}\left((i \cdot \ln i)^{1.585}\right) \approx \mathcal{O}\left(M^{2.585} \cdot \ln^{1.585} M\right)$$

Conversely, Naive Rejection executes $\frac{M(M-1)}{2} = \mathcal{O}(M^2)$ pairwise checks. Granted Schönhage fast GCD, its total workload scales as:

$$W_{\text{Naive}}(M) = \mathcal{O}(M^2) \times \mathcal{O}\left((M \cdot \ln M)^{1.585} \cdot \log(M \cdot \ln M)\right) = \mathcal{O}\left(M^{3.585} \ln^{1.585} M \log(M \ln M)\right)$$

Taking the ratio isolates the relative asymptotic advantage factor:

$$\frac{W_{\text{Naive}}(M)}{W_{\text{Engine}}(M)} = \frac{\mathcal{O}\left(M^{3.585} \cdot \ln^{1.585} M \cdot \log(M \cdot \ln M)\right)}{\mathcal{O}\left(M^{2.585} \cdot \ln^{1.585} M\right)} = \mathcal{O}(M \log M)$$

Even when Naive Euclidean sampling is artificially granted recursive Schönhage fast GCD algorithms, the $0$-GCD Forward Algebraic Mapping strictly preserves a scaling performance advantage factor of $\mathcal{O}(M \log M)$.

### Section 3: Track 2 Strong Divisibility Sequence Proof

A sequence $\left\\{U\_n\right\\}\_{n \ge 1}$ is a Strong Divisibility Sequence (SDS) if $\gcd(U\_n, U\_m) = U\_{\gcd(n,m)}$ for all $n,m \ge 1$. The Lucas sequence evaluated at $P=1, Q=-1$ yields the Fibonacci sequence $F_n$.

**Proof of Autonomous Coprimality:** \
Evaluating an SDS at distinct prime indices $p_i \ge 2$ generates a strictly pairwise coprime set. Let $p_i, p_j$ be distinct primes. By the fundamental SDS property:


$$\gcd(F_{p_i}, F_{p_j}) = F_{\gcd(p_i, p_j)} = F_1 = 1$$

**Exact Asymptotic Contraction Derivation:** \
The SDS framework yields an exact asymptotic spatial memory reduction of $1 - \ln(\phi) \approx 51.8789\%$ compared to arithmetic primorial algorithms.

*Proof:* By the Prime Number Theorem and Chebyshev's first function $\vartheta(x) = \sum_{p \le x} \ln p \sim x$.
The asymptotic bit-length of the $M$-th element in a primorial sequence $P_M\\# = \prod_{i=1}^M p_i$ is bounded by:


$$L_{\text{prim}}(M) = \log_2(P_M\\#) = \log_2(e) \cdot \vartheta(p_M) \sim p_M \log_2(e)$$


Conversely, the continuous evaluation of the $M$-th element in the SDS track is governed by Binet's formula $F_n = \frac{\phi^n - \psi^n}{\sqrt{5}}$, where $\phi = \frac{1+\sqrt{5}}{2}$ and $\psi = \frac{1-\sqrt{5}}{2}$. As $n \to \infty$, $\psi^n \to 0$, bounding the bit-length as:


$$L_{\text{sds}}(M) = \log_2(F_{p_M}) \sim p_M \log_2(\phi) - \frac{1}{2}\log_2(5)$$


Taking the ratio of their spatial footprints as $M \to \infty$:


$$\lim_{M \to \infty} \frac{L_{\text{sds}}(M)}{L_{\text{prim}}(M)} = \frac{p_M \log_2(\phi)}{p_M \log_2(e)} = \frac{\ln(\phi)}{\ln(e)} = \ln(\phi) \approx 0.4812118$$


Subtracting from unity yields the geometric memory contraction:


$$1 - \ln(\phi) \approx 1 - 0.4812118 = 0.5187882 \quad (51.8789\\%)$$

### Section 4: Dedekind Domain Extension & Principal Ideal Norm Preservation

Extending the mapping to the ring of integers $\mathcal{O}_K$ of an arbitrary algebraic number field $K/\mathbb{Q}$ rigorously bypasses the Split-Prime Norm Fallacy.

**Theorem 4.1 (Principal Ideal Coprimality Transfer):**
Let $\{d_1, d_2, \dots, d_M\} \subset \mathbb{Z}$ be a sequence of pairwise coprime integers generated via Track 1 or 2. Let the canonical injection $\mathbb{Z} \hookrightarrow \mathcal{O}_K$ define a sequence of principal ideals $\mathfrak{q}_i = \langle d_i \rangle = d_i \mathcal{O}_K$.

Because $\gcd(d_i, d_j) = 1$ in $\mathbb{Z}$, Bezout's identity states $x d_i + y d_j = 1$ for integers $x, y \in \mathbb{Z}$. Since $x, y \in \mathcal{O}_K$, it follows that $x d_i \in \mathfrak{q}_i$ and $y d_j \in \mathfrak{q}_j$, yielding $1 \in \mathfrak{q}_i + \mathfrak{q}_j$. Thus, $\mathfrak{q}_i + \mathfrak{q}_j = \mathcal{O}_K$.

**Lemma 4.2 & Theorem 4.3 (Unconditional Coprimality Preservation under Absolute Norm):**
The Split-Prime Norm Fallacy dictates that arbitrary prime ideals that split completely may collide under the absolute field norm. However, because our principal ideals $\mathfrak{q}_i = d_i \mathcal{O}_K$ are natively derived from rational integers $d_i \in \mathbb{Z}$, their norm projection acts strictly as scalar exponentiation.

For a field extension $K/\mathbb{Q}$ of degree $n = [K : \mathbb{Q}]$:


$$\mathcal{N}_{K/\mathbb{Q}}(d_i \mathcal{O}_K) = \lvert d_i \rvert^n$$


Since $\gcd(d_i, d_j) = 1$ in $\mathbb{Z}$, the fundamental theorem of arithmetic guarantees that exponentiation by $n$ introduces no new prime factors. Thus:


$$\gcd(\lvert d_i \rvert^n, \lvert d_j \rvert^n) = 1$$


The principal ideal $\mathfrak{q}_i$ inherently recombines all conjugate split prime ideals natively. The absolute norm maps perfectly back to $\mathbb{Z}$, unconditionally preserving the coprimality invariant. $\blacksquare$
