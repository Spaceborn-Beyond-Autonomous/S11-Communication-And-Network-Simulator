# S11 — Communication & Network Simulator

S11 is a protocol-independent C simulator that turns generic/test traffic into realistic network traffic by applying configurable latency, jitter, packet loss and bandwidth conditions. It also models link failures, network partitions, jamming, recovery, logging and terminal monitoring.

This implementation follows the supplied S11 architecture document: generic/test traffic is used first so the project does not wait for S10/S05, while clean protocol boundaries remain available for ROS2, DDS and MAVLink integration.

## 1. What is included

- Generic protocol-independent packet API
- Configurable link profiles in `config/network.yaml`: LTE, 5G, mesh, LoRa and satellite
- Latency, jitter, packet-loss and bandwidth models
- Link states: UP, DOWN, DEGRADED, PARTITIONED and JAMMED
- Partition, recovery and jamming events
- ROS2, DDS and MAVLink adapter boundaries
- Runtime packet/event logging to `logs/s11_network.log`
- Terminal monitor/dashboard generated from the actual runtime network state
- CMake build and executable

The architecture document requires configurable degradation, forwarding/drop decisions, partition/recovery, jamming representation, logs, monitoring and a public API suitable for later S10/S05 integration.

## 2. Project structure

```text
S11-Communication-And-Network-Simulator/
├── include/                 # Shared public APIs
├── src/core/                # Packet/network/config integration
├── src/models/              # Latency/loss/jitter/bandwidth/jamming
├── src/protocols/           # ROS2/DDS/MAVLink boundaries
├── src/topology/            # Links, partition, recovery
├── src/logger/              # Runtime event and packet logging
├── src/monitor/             # Monitor + dynamic dashboard
├── config/                  # Network and scenario configuration
├── tests/                   # Integration tests
├── docs/                    # Architecture/validation documentation
├── logs/                    # Runtime logs
├── CMakeLists.txt
└── README.md
```

## 3. Requirements

Ubuntu/Linux with:

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config libyaml-dev
```

## 4. Build the project

From the project root:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

The executable is:

```text
build/s11_simulator
```

## 5. Run the complete simulator

The dashboard is now part of the C program. **There is no separate `dashboard.sh` dependency.** The dashboard reads the actual network counters, link state, selected link configuration and simulation time produced by the running simulator.

Run the complete scenario demonstration:

```bash
./build/s11_simulator
```

This executes the available demonstration scenarios and then prints the runtime dashboard.

## 6. Run one scenario

```bash
./build/s11_simulator --scenario normal
./build/s11_simulator --scenario high_latency
./build/s11_simulator --scenario high_packet_loss
./build/s11_simulator --scenario unstable
./build/s11_simulator --scenario low_bandwidth
./build/s11_simulator --scenario partition
./build/s11_simulator --scenario recovery
./build/s11_simulator --scenario jamming
```

Use `--help` to see all options:

```bash
./build/s11_simulator --help
```

## 7. Select the network link profile

The link profile is loaded from `config/network.yaml`:

```bash
./build/s11_simulator --link lte
./build/s11_simulator --link 5g
./build/s11_simulator --link mesh
./build/s11_simulator --link lora
./build/s11_simulator --link satellite
```

You can combine options, for example:

```bash
./build/s11_simulator --link satellite --scenario high_latency --packets 5
```

The dashboard will display the selected link type and the values loaded from the configuration file.

## 8. How the dashboard works

The dashboard is implemented in `src/monitor/network_dashboard.c` and linked directly into `s11_simulator` through `CMakeLists.txt`.

It is **not a hard-coded report**. The following values are calculated from the current simulator run:

- RX packet count
- TX packet count
- DROP packet count
- Average forwarding delay
- Loss rate
- Total accumulated delay
- Current link state
- Link type
- Base latency
- Jitter range
- Packet-loss configuration
- Bandwidth
- Queue capacity
- Simulation time
- Selected scenario/network ID

Therefore changing the YAML configuration, link type, scenario or packet count changes the resulting dashboard.

## 9. Runtime logs

The logger writes packet and topology events to:

```bash
cat logs/s11_network.log
```

Useful commands:

```bash
less logs/s11_network.log
tail -f logs/s11_network.log
```

Each run appends to the log. To start a fresh run:

```bash
: > logs/s11_network.log
./build/s11_simulator
```

## 10. Run tests

After building, run the registered CTest suite:

```bash
ctest --test-dir build --output-on-failure
```

The suite currently covers:

- `s11_smoke` — normal packet-processing path
- `s11_partition_recovery` — partition/drop path
- `s11_jamming` — jamming/drop path

CTest is configured to run from the project source directory so `config/network.yaml` and `logs/` resolve correctly.

If the project has not been configured yet, run the build commands first.

## 11. Recommended complete workflow

```bash
cd ~/S11-Communication-And-Network-Simulator-complete/S11-Communication-And-Network-Simulator

cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
: > logs/s11_network.log
./build/s11_simulator
```

## 12. How the data flows

```text
Generic/Test Packet
       │
       ▼
Packet Manager
       │
       ▼
Network Manager
       │
       ├── Link State / Topology
       ├── Latency Model
       ├── Jitter Model
       ├── Packet Loss Model
       └── Bandwidth Model
       │
       ├──────────────► Forwarded
       │
       └──────────────► Dropped
              │
              ├── Logger → logs/s11_network.log
              └── Monitor/Dashboard → terminal
```

## 13. Team architecture

The supplied plan assigns Chetanya to core/integration, Aya to network models, Ashutosh to protocol adapters, and Hemant to topology/failure/observability. The modules communicate through shared headers rather than depending directly on another member's internal implementation.

## 14. Important scope boundary

The MVP deliberately does not wait for S10/S05, does not implement physical LTE/5G/LoRa/satellite radio physics, does not implement real cybersecurity attacks, does not tightly couple S11 to one application, and does not create a separate core packet format for each protocol. These boundaries are part of the supplied project plan.

## 15. Final acceptance checklist

- [x] CMake project builds
- [x] Generic packet injection
- [x] Configurable latency, jitter, packet loss and bandwidth
- [x] Forward/drop decisions
- [x] Partition and recovery representation
- [x] Jamming/link-state representation
- [x] Timestamp/source/destination/protocol/status/metric logging
- [x] Terminal monitoring/dashboard
- [x] Protocol-independent public API
- [x] Documentation and operation instructions
- [x] CTest tests registered and runnable

The acceptance criteria above follow the supplied S11 project document.
