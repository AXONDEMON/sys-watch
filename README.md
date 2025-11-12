<p align="center">
  <img src="https://readme-typing-svg.demolab.com?font=Fira+Code&size=24&duration=3000&pause=1000&color=0AFFEF&center=true&vCenter=true&width=600&lines=SysWatch+-+Multi-threaded+System+Monitor;Real-time+Server+Health+Monitoring;POSIX+Threads+%7C+Signal+Handling+%7C+I/O+Multiplexing" alt="Typing SVG" />
</p>

<h1 align="center">🖥️ SysWatch – Dynamic System Monitoring Tool</h1>

<p align="center">
  <b>A multi-threaded system monitoring daemon built in C using POSIX threads, synchronization primitives, and signal handling.</b><br>
  Designed to monitor CPU, memory, disk, and logs in real-time for mission-critical servers handling financial workloads.
</p>

---

## 📘 Overview

**SysWatch** is a lightweight, extensible monitoring tool that continuously observes key system parameters and logs in real time.  
It demonstrates concepts in **multi-threading, concurrency control, I/O multiplexing, signal handling, and graceful process management** in a POSIX environment.

This implementation was built as part of a **System Programming & Operating Systems** assignment for developing a concurrent monitoring daemon.

---

## ⚙️ Features

✅ **Multi-threaded core** – Separate threads handle CPU/memory, logs, and networking  
✅ **Mutex-protected shared state** – Ensures safe concurrent metric updates  
✅ **Signal handling (SIGTERM, SIGUSR1, SIGHUP)** – Graceful shutdown and on-demand metrics dump  
✅ **Config-driven architecture** – Easy to add new monitored paths or log files  
✅ **I/O multiplexing (poll)** – Efficient log file monitoring  
✅ **Rolling metrics log** – Keeps a history of sampled system metrics  
✅ **Shell wrapper for automation** – Optional script (`scripts/syswatch.sh`) for remote or batch execution  

---

## 🧩 Project Structure
SysWatch/
│
├── include/
│   └── syswatch.h            # Global structs, macros, and function declarations
│
├── src/
│   ├── syswatch.c            # Main process – spawns threads & handles signals
│   ├── metrics.c             # Metrics monitoring thread
│   ├── logs.c                # Log watcher thread
│   ├── net.c                 # Network status thread
│   └── util.c                # Utility helpers (timestamps, sleep)
│
├── config/
│   └── syswatch.conf         # Configuration file (mounts, logs, ports)
│
├── scripts/
│   └── syswatch.sh           # Shell wrapper for remote or multi-node execution
│
├── Makefile                  # Build system
└── README.md                 # Project documentation
---

## 🧱 Build & Run

### 🔧 Prerequisites
- macOS or Linux (POSIX-compliant system)
- GCC or Clang
- Make

### 🏗️ Build
```bash
make clean
make -B

# Sample SysWatch configuration
mount=/dev/disk1s5
log=/var/log/system.log
metrics_log=syswatch_metrics.log
sample_interval=5
bind_addr=0.0.0.0
bind_port=9090
