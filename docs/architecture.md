# S11 Runtime Architecture

```text
Generic/Test traffic
        |
        v
  packet_manager
        |
        v
  network_manager ---- topology/link_manager
        |
        +---- packet loss
        +---- latency
        +---- jitter
        +---- bandwidth
        |
        +---- logger -> logs/s11_network.log
        +---- monitor/dashboard -> terminal
```

The packet representation remains protocol-independent. ROS2, DDS and MAVLink are adapter boundaries. Network models are selected from `config/network.yaml`; runtime scenario modifiers are applied by the simulator before a packet enters the common network processing pipeline.

The dashboard is compiled into the simulator and consumes runtime counters and configuration rather than a separate hard-coded output script.
