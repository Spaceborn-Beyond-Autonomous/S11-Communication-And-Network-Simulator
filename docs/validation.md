# S11 Validation

## Build

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## Automated smoke test

```bash
ctest --test-dir build --output-on-failure
```

The smoke test runs the real `s11_simulator` executable against the normal scenario.

## Manual validation

Run:

```bash
./build/s11_simulator
./build/s11_simulator --scenario high_latency --packets 5
./build/s11_simulator --scenario partition
./build/s11_simulator --scenario jamming
./build/s11_simulator --link satellite --scenario high_latency --packets 3
```

Check that the terminal dashboard changes with packet count, link type, scenario and configuration. Also inspect `logs/s11_network.log` for packet and topology events.
