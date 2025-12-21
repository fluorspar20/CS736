# An Exploratory Study of ALPS Scheduler for I/O and CPU-Bound Serverless Workloads

This repository contains a custom implementation and performance analysis of the **ALPS (Adaptive Learning, Priority Scheduler)**, adapted for modern Linux environments using the **Google ghOSt (Generic Hosted Operating System Scheduler)** framework and **eBPF**.

This project reproduces the core findings of the original ALPS paper for CPU-bound workloads and extends the evaluation to mixed I/O-heavy scenarios, identifying critical "blind spots" in CPU-centric learning policies.

## 📖 Overview

Serverless (FaaS) workloads are characterized by short lifespans, high concurrency, and extreme burstiness. Traditional schedulers like Linux CFS (Completely Fair Scheduler) prioritize fairness over latency, often causing high tail latencies for short functions due to context switching overhead.

ALPS aims to solve this by approximating **Shortest Remaining Processing Time (SRPT)**. It "learns" function durations from historical traces to prioritize shorter jobs.

**Our Contribution:**
1. **Modern Implementation:** Ported the ALPS logic to **Google ghOSt** and **eBPF** to bypass deprecated Docker dependencies in the original artifact.
2. **I/O "Blind Spot" Analysis:** Diagnosed a fundamental flaw where I/O-bound tasks are misclassified as "short jobs" (due to low CPU usage), causing priority inversion and starvation of concurrent CPU-bound tasks.

## 🏗 Architecture

The scheduler follows a split-level architecture, decoupling policy decision-making (userspace) from policy enforcement (kernel).

### 1. Frontend (Userspace Policy Engine)
* **Framework:** Google ghOSt Userspace Agent.
* **Logic:** Implements a sliding window mechanism to track historical function execution times.
* **Learning:** Uses **Linear Regression** (and SRPT simulation) on live traces to predict future runtime and assign dynamic time slices/priorities.
* **Mechanism:** Updates shared eBPF maps with task priorities (Rank) and Time Slices.

### 2. Backend (Kernel Enforcement)
* **Framework:** Linux Kernel with ghOSt patches + eBPF hooks.
* **Enforcement:** Intercepts scheduling events via `alps_execve()` to tag tasks with unique function IDs.
* **Scheduling:** Manipulates task `vruntime` in the CFS Red-Black tree based on the priority retrieved from eBPF maps.

## 🔧 Implementation Details

Due to version mismatches in the original ALPS artifact (Docker v17.06 dependency), we re-engineered the system on **CloudLab c6420 nodes** running **Ubuntu 20.04**.

* **Hardware:** Intel Xeon Gold 6142 (16 cores), 384 GB RAM.
* **Kernel:** Custom ghOSt-enabled Linux kernel.
* **Workloads:**
    * **CPU-Bound:** Fibonacci sequence calculations (replicating original ALPS study).
    * **I/O-Bound:** Synthetic functions simulating DB queries (Sleep → Compute bursts).
    * **Mixed:** Concurrent execution of both classes to stress test priority logic.

## 📊 Evaluation & Results

### 1. CPU-Bound Replication (Baseline)
We successfully reproduced the original ALPS results. By approximating SRPT, our implementation significantly reduced the Average Function Completion Time (FCT) for short, CPU-intensive tasks compared to CFS.

### 2. The I/O "Blind Spot" (Mixed Workloads)
Our experiments revealed that ALPS struggles to differentiate between "true" short jobs and long-running jobs that sleep frequently (I/O).

* **Observation:** I/O-bound tasks consume very little CPU time between waits. The scheduler incorrectly learns them as "High Priority / Short Jobs".
* **Pathology:** This leads to **Priority Inversion**. I/O tasks aggressively preempt CPU tasks every time they wake up, causing context switch storms.
* **Impact:**
    * **I/O Performance:** P99 Latency **reduced by ~30%** (vs CFS) because they are treated as VIPs.
    * **CPU Performance:** Throughput **degraded by 15%**; Tail latency **increased by 13-15%** due to starvation.

The final project report is attached along with the other files.
