#!/usr/bin/env python3

import json
import matplotlib.pyplot as plt
import os


def generate_dashboard():
    telemetry_file = "telemetry.json"

    if not os.path.exists(telemetry_file):
        print(f"[!] Error: {telemetry_file} not found. Run the coprime_inspector first.")
        return

    with open(telemetry_file, "r") as f:
        data = json.load(f)

    # 1. Parse strict telemetry invariants
    M = [item["M"] for item in data]
    t1_lat = [item["T1_latency_us"] for item in data]
    t2_lat = [item["T2_latency_us"] for item in data]
    naive_lat = [item["Naive_latency_us"] for item in data]

    t1_bits = [item["T1_total_bits"] for item in data]
    t2_bits = [item["T2_total_bits"] for item in data]
    naive_bits = [item["Naive_total_bits"] for item in data]
    naive_gcds = [item["Naive_GCDs"] for item in data]

    # 2. REMEDIATION: Strict Bit-Throughput Derivation (Mbps)
    # Total spatial bits / (latency in seconds) / 1,000,000
    t1_mbps = [b / (lat / 1e6) / 1e6 for b, lat in zip(t1_bits, t1_lat)]
    t2_mbps = [b / (lat / 1e6) / 1e6 for b, lat in zip(t2_bits, t2_lat)]
    naive_mbps = [b / (lat / 1e6) / 1e6 for b, lat in zip(naive_bits, naive_lat)]

    # 3. Canvas and UI Framework Initialization
    plt.style.use('dark_background')
    fig, axs = plt.subplots(2, 2, figsize=(14, 10), dpi=300)
    fig.patch.set_facecolor('#0d1117')

    for ax in axs.flat:
        ax.set_facecolor('#161b22')
        ax.grid(True, linestyle='--', alpha=0.3, color='#8b949e')

    # Panel 1: Execution Latency Pipeline (Log-Log)
    axs[0, 0].loglog(M, naive_lat, 'o-', color='#f85149', label='Naive Sampler O(M²)')
    axs[0, 0].loglog(M, t1_lat, 's-', color='#58a6ff', label='Track 1 (Cyclotomic)')
    axs[0, 0].loglog(M, t2_lat, '^--', color='#3fb950', label='Track 2 (Fibonacci SDS)')
    axs[0, 0].set_title("Execution Latency (µs) vs Scale M", color='#c9d1d9', fontsize=12, fontweight='bold')
    axs[0, 0].set_xlabel("Set Size M", color='#8b949e')
    axs[0, 0].set_ylabel("Latency (µs)", color='#8b949e')
    axs[0, 0].legend()

    # Panel 2: Exact Spatial Bit-Throughput (Log-Log)
    axs[0, 1].loglog(M, t1_mbps, 's-', color='#58a6ff', label='Track 1 (Cyclotomic)')
    axs[0, 1].loglog(M, t2_mbps, '^--', color='#3fb950', label='Track 2 (Fibonacci SDS)')
    axs[0, 1].loglog(M, naive_mbps, 'o-', color='#f85149', label='Naive Sampler')
    axs[0, 1].set_title("Exact Spatial Bit-Throughput (Mbps) vs Scale M", color='#c9d1d9', fontsize=12,
                        fontweight='bold')
    axs[0, 1].set_xlabel("Set Size M", color='#8b949e')
    axs[0, 1].set_ylabel("Throughput (Mbps)", color='#8b949e')
    axs[0, 1].legend()

    # Panel 3: Exact Triangular Spatial Mass Integration (Log-Log)
    axs[1, 0].loglog(M, t1_bits, 's-', color='#58a6ff', label='Track 1 Mass (Bits)')
    axs[1, 0].loglog(M, t2_bits, '^--', color='#3fb950', label='Track 2 Mass (Bits)')
    axs[1, 0].loglog(M, naive_bits, 'o-', color='#f85149', label='Naive Mass (Bits)')
    axs[1, 0].set_title("Exact Triangular Spatial Mass (Total Bits Generated)", color='#c9d1d9', fontsize=12,
                        fontweight='bold')
    axs[1, 0].set_xlabel("Set Size M", color='#8b949e')
    axs[1, 0].set_ylabel("Cumulative Bit-Width", color='#8b949e')
    axs[1, 0].legend()

    # Panel 4: Asymptotic Branching Bottleneck (Linear-Log)
    axs[1, 1].plot(M, naive_gcds, 'o-', color='#f85149', label='Naive Sampler O(M²)')
    axs[1, 1].axhline(y=0, color='#58a6ff', linewidth=2, label='Engine Hardware Invariant (0 GCDs)')
    axs[1, 1].set_title("Euclidean GCD Calls Executed", color='#c9d1d9', fontsize=12, fontweight='bold')
    axs[1, 1].set_xlabel("Set Size M", color='#8b949e')
    axs[1, 1].set_ylabel("GCD Calls", color='#8b949e')
    axs[1, 1].legend()

    plt.tight_layout()
    plt.savefig("../docs/assets/benchmark_dash.svg", facecolor=fig.get_facecolor(), edgecolor='none')
    print("[+] Dashboard rendered successfully: 'benchmark_dash.svg'")


if __name__ == "__main__":
    generate_dashboard()
