# S11 — Communication & Network Simulator

**Member 1 — Core & Integration — Day 1 Report**

> This README documents the **Day 1** contribution of Member 1 (Core & Integration) only. It does not describe the final state of the full S11 project, and it does not claim ownership of modules belonging to other team members.

---

## 1. Project Overview

S11 is a planned **Layer-8 Communication & Network Simulator**. Its goal is to simulate generic communication traffic and apply configurable network conditions on top of it, so that higher-level systems can be tested against realistic (and adverse) network behavior without depending on real hardware or live networks.

The project is being built by a team of four members, each owning a distinct module boundary. This document covers only the portion of the system owned and delivered by **Member 1**.

## 2. S11 Purpose

S11 simulates generic communication traffic and applies configurable network conditions such as:

- Latency
- Jitter
- Packet loss
- Bandwidth limitations
- Link failures
- Jamming
- Network partitions
- Recovery

The simulator core is intentionally **protocol-independent**, so that it can later carry traffic from:

- ROS 2
- DDS
- MAVLink

The MVP is designed to run against generic/test traffic first, and must not depend on unfinished external projects (e.g., S10, S15).

## 3. Day 1 Objective

Member 1's Day 1 objective, as defined by the project plan, was:

> Freeze packet structure, network-state structure, and public APIs; create the CMake project and integration skeleton.

**Day 1 deliverable:** all folders compile, and the shared header/API contracts are agreed upon across the team.

## 4. Member 1 Responsibilities

Member 1 (Core & Integration) is responsible for:

- Core architecture of the simulator
- Packet lifecycle definition
- Network lifecycle definition
- The packet processing entry point (`s11_network_process`)
- Configuration integration (`config_manager.c`)
- Integration between all team modules
- Final merge/integration of the project

Member 1 owns the following files:

```text
include/
├── s11_packet.h
└── s11_network.h

src/core/
├── network_manager.c
├── packet_manager.c
└── config_manager.c

src/main.c
CMakeLists.txt
```

> **Note:** Skeleton versions of `s11_protocol.h`, `s11_link.h`, `s11_events.h`, and `s11_common.h` currently exist in the repository so that the project can compile end-to-end on Day 1. These headers are **shared contracts**, and their final API ownership belongs to Member 3 (Protocols) and Member 4 (Topology/Failure/Observability) respectively, as described in [Module Ownership](#10-module-ownership). Nothing in this document should be read as Member 1 claiming implementation credit for those modules.

## 5. Architecture Overview

The current repository structure is:

```text
s11_communication_network/
├── CMakeLists.txt
├── config/
│   ├── network.yaml
│   └── scenarios.yaml
├── docs/
├── include/
│   ├── s11_common.h
│   ├── s11_events.h
│   ├── s11_link.h
│   ├── s11_network.h
│   ├── s11_packet.h
│   └── s11_protocol.h
├── logs/
├── src/
│   ├── core/
│   │   ├── config_manager.c
│   │   ├── network_manager.c
│   │   └── packet_manager.c
│   ├── logger/
│   │   └── network_logger.c
│   ├── models/
│   │   ├── bandwidth_model.c
│   │   ├── jamming_model.c
│   │   ├── jitter_model.c
│   │   ├── latency_model.c
│   │   └── packet_loss_model.c
│   ├── monitor/
│   │   └── network_monitor.c
│   ├── protocols/
│   │   ├── dds_adapter.c
│   │   ├── mavlink_adapter.c
│   │   └── ros2_adapter.c
│   ├── topology/
│   │   ├── link_manager.c
│   │   ├── partition_manager.c
│   │   └── recovery_manager.c
│   └── main.c
└── tests/
```

Architecturally, the system is organized as:

- **Core (Member 1):** owns the packet and network data structures, the processing entry point, configuration integration, and overall build/integration.
- **Network Models (Member 2):** will implement latency, jitter, packet loss, bandwidth, and jamming behavior.
- **Protocols/Adapters (Member 3):** will implement protocol identification and conversion for ROS 2, DDS, and MAVLink.
- **Topology/Failure/Observability (Member 4):** will implement link/topology management, partitions, recovery, logging, and monitoring.

Modules interact only through the shared public headers in `include/`, not through each other's internal source files.

## 6. Packet Architecture

The packet is the single, protocol-independent unit of data exchanged in the simulator.

```c
typedef struct
{
    uint64_t packet_id;
    char source[S11_NODE_ID_MAX_LEN];
    char destination[S11_NODE_ID_MAX_LEN];
    s11_protocol_t protocol;
    uint8_t *payload;
    uint32_t payload_size;
    uint64_t timestamp_us;
} s11_packet_t;
```

Design decisions:

- The packet is generic and protocol-independent — there is one packet representation for ROS 2, DDS, and MAVLink traffic, not three.
- `source` and `destination` identify the sending and receiving nodes.
- `protocol` records which communication protocol produced the packet (or `S11_PROTOCOL_UNKNOWN`).
- `payload` holds the raw bytes of the message body.
- The packet **owns** its payload memory: `s11_packet_create` copies the caller's payload, and `s11_packet_destroy` frees it.
- `payload_size` records the payload length in bytes.
- `timestamp_us` is **simulation time** in microseconds, not wall-clock time.
- Packet memory management is the responsibility of the packet manager (`packet_manager.c`), not of individual protocol adapters or models.

## 7. Network State Architecture

The network structure represents the **runtime state** of the simulation, not its configuration.

```c
typedef struct
{
    char network_id[S11_NETWORK_ID_MAX_LEN];
    uint64_t packets_received;
    uint64_t packets_transmitted;
    uint64_t packets_dropped;
} s11_network_t;
```

By design, `s11_network_t` does **not** contain:

- Latency, jitter, packet-loss, or bandwidth parameters
- Link/topology information
- Protocol-specific internals

Those fields belong to their respective owning modules (Member 2's models and Member 4's topology layer), not to the core network state.

Packet processing produces a result describing what happened to the packet:

```c
typedef enum
{
    S11_PACKET_FORWARDED = 0,
    S11_PACKET_DROPPED
} s11_process_status_t;

typedef struct
{
    s11_process_status_t status;
    uint64_t delay_us;
} s11_process_result_t;
```

`network_manager.c` is intended to act as an **orchestrator**: it calls into the packet, model, protocol, and topology modules in sequence, but it does not itself implement latency, jitter, loss, bandwidth, topology, or logging logic.

## 8. Public Core APIs

### Packet API (`s11_packet.h`)

```c
s11_packet_t *s11_packet_create(
    const char *source,
    const char *destination,
    s11_protocol_t protocol,
    const uint8_t *payload,
    uint32_t payload_size,
    uint64_t timestamp_us
);

void s11_packet_destroy(s11_packet_t *packet);

bool s11_packet_validate(const s11_packet_t *packet);
```

| Function | Responsibility |
|---|---|
| `s11_packet_create` | Allocates a new packet and copies the given payload into it. Caller keeps ownership of the input buffer; the created packet owns its internal copy. |
| `s11_packet_destroy` | Frees a packet and its internally owned payload. |
| `s11_packet_validate` | Checks whether a packet's fields are valid before it is processed further. |

### Network API (`s11_network.h`)

```c
s11_network_t *s11_network_create(const char *network_id);

void s11_network_destroy(s11_network_t *network);

s11_process_result_t s11_network_process(
    s11_network_t *network,
    const s11_packet_t *packet
);
```

| Function | Responsibility |
|---|---|
| `s11_network_create` | Allocates and initializes a network's runtime state. |
| `s11_network_destroy` | Releases a network's resources. |
| `s11_network_process` | Entry point for processing a single packet through the network; returns whether it was forwarded or dropped, and the delay applied. |

These are the only APIs currently defined for the Core module. No additional functions have been added beyond what exists in the headers today.

## 9. Packet Lifecycle

The following is the **planned** high-level lifecycle a packet is expected to go through once all modules are implemented. On Day 1, this describes the intended architecture — it is not a claim that every stage below is functionally implemented yet.

```text
CREATE
   ↓
VALIDATE
   ↓
INJECT
   ↓
CHECK LINK
   ↓
APPLY LOSS
   ↓
CALCULATE DELAY
   ↓
APPLY JITTER
   ↓
CHECK BANDWIDTH
   ↓
FORWARD / DROP
   ↓
UPDATE METRICS
   ↓
LOG EVENT
```

- **CREATE / VALIDATE:** handled today by `s11_packet_create` / `s11_packet_validate` (Member 1).
- **INJECT / CHECK LINK / APPLY LOSS / CALCULATE DELAY / APPLY JITTER / CHECK BANDWIDTH:** will be orchestrated by `network_manager.c`, calling into Member 2's models and Member 4's topology layer once those are implemented.
- **FORWARD / DROP / UPDATE METRICS:** reflected in `s11_process_result_t` and the counters in `s11_network_t`.
- **LOG EVENT:** will be handled by Member 4's logging module.

## 10. Module Ownership

| Member | Owns | Responsibilities |
|---|---|---|
| **Member 1 — Core & Integration** | `s11_packet.h`, `s11_network.h`, `network_manager.c`, `packet_manager.c`, `config_manager.c`, CMake/integration | Core architecture, packet lifecycle, network lifecycle, packet processing entry point, configuration integration, cross-module integration, final merge |
| **Member 2 — Network Models** | `latency_model.c`, `packet_loss_model.c`, `jitter_model.c`, `bandwidth_model.c`, `jamming_model.c` | Network-condition models, common model interface, model configuration fields |
| **Member 3 — Protocols/Adapters** | `s11_protocol.h`, `ros2_adapter.c`, `dds_adapter.c`, `mavlink_adapter.c` | Protocol abstraction, protocol adapters, packet conversion |
| **Member 4 — Topology/Failure/Observability** | `s11_link.h`, `s11_events.h`, `link_manager.c`, `partition_manager.c`, `recovery_manager.c`, `network_logger.c`, `network_monitor.c` | Link/topology management, partition/recovery, failure events, logging, monitoring |

All modules communicate exclusively through the shared headers in `include/`. Internal implementation files of one module are not accessed directly by another.

## 11. Architecture Principles

1. **Module-first architecture** — each member owns a clear, non-overlapping module boundary.
2. **Shared headers define contracts** — modules communicate through public interfaces rather than reaching into each other's internal implementation.
3. **Generic packet format** — there is one common packet representation, not separate packet structures for ROS 2, DDS, and MAVLink.
4. **Separation of configuration and runtime state** — configuration describes the selected scenario and network conditions; runtime state describes what is currently happening during simulation.
5. **Separation of concerns** — `network_manager.c` orchestrates processing but does not itself implement latency, jitter, loss, bandwidth, topology, logging, or protocol conversion.
6. **MVP independence** — the simulator can be exercised with generic/test traffic without waiting on S10 or S15.

## 12. Current Day 1 Status

- ✅ Packet structure (`s11_packet_t`) frozen and documented.
- ✅ Network state structure (`s11_network_t`) frozen and documented.
- ✅ Public Core APIs for packet and network management defined.
- ✅ CMake project created; all current folders/files compile successfully.
- ✅ Integration skeleton (`src/main.c`) runs and prints a smoke-test message.
- ✅ Shared header contracts (`s11_protocol.h`, `s11_link.h`, `s11_events.h`, `s11_common.h`) exist as initial skeletons to unblock compilation; final ownership remains with Members 3 and 4.
- ⏳ Latency, jitter, packet loss, bandwidth, and jamming models are **not yet implemented** — they exist only as source-file placeholders in the build (Member 2, future work).
- ⏳ Protocol adapters for ROS 2, DDS, and MAVLink are **not yet implemented** (Member 3, future work).
- ⏳ Link/topology, partitioning, recovery, logging, and monitoring are **not yet implemented** (Member 4, future work).

The current achievement for Day 1 is a **compilable integration skeleton** with frozen core contracts — not a functioning end-to-end simulator.

## 13. Build Instructions

Environment used during Day 1 development:

- CMake 3.28.3
- GCC 13.3.0
- Ubuntu 24.04
- C17

Build steps:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

## 14. Run Instructions

From the build directory:

```bash
./s11_simulator
```

## 15. Expected Output

```text
S11 Communication & Network Simulator
Integration skeleton is working.
```

This confirms that the project compiles as a whole and that the integration skeleton executes correctly. It does not indicate that packet processing, network conditions, or protocol adapters are functional yet.

## 16. Future Work / Day 2

- Implement the packet manager's internal logic (`packet_manager.c`) against the frozen `s11_packet_t` API.
- Implement `network_manager.c` as an orchestrator that calls into Member 2's models and Member 4's topology layer.
- Implement `config_manager.c` to load and apply `config/network.yaml` and `config/scenarios.yaml`.
- Coordinate with Member 2, 3, and 4 as they finalize their owned headers (`s11_protocol.h`, `s11_link.h`, `s11_events.h`) so Core can integrate against stable contracts.
- Extend the smoke test in `src/main.c` into real integration tests as modules become available.

## 17. Integration Notes

- All cross-module communication must go through the public headers in `include/`, not through direct calls into another member's `.c` files.
- The skeleton versions of `s11_protocol.h`, `s11_link.h`, and `s11_events.h` currently in the repository exist only so the Day 1 build compiles; they are expected to be revised by Members 3 and 4 as their modules mature, and Core will adapt to those changes.
- Any change to `s11_packet_t`, `s11_network_t`, or the Core public APIs described in this document should be communicated to the whole team, since other modules will be built against these contracts.
- The build currently compiles all source files listed in `CMakeLists.txt`, including placeholder files for modules not yet implemented, to keep the whole team unblocked and building against the same skeleton.