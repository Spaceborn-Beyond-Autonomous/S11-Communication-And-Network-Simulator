# S11 Header Changes — Network Models (Aya)

## Files touched
- `s11_network.h`
- `s11_link.h`

## Added to `s11_network.h`

**`s11_link_model_config_t`** — per-link degradation config (latency, jitter, loss, bandwidth), one instance per link type, loaded from `config/network.yaml`. Fields: `link_type`, `base_latency_ms`, `jitter_min_ms`, `jitter_max_ms`, `packet_loss_rate`, `bandwidth_bps`, `queue_capacity`.

**`s11_model_apply_fn`** — common function pointer signature for the four degradation models (latency, jitter, packet loss, bandwidth). Reuses `s11_process_result_t` as the return type — no new result struct introduced.

## Added to `s11_link.h`

**`link_type[16]`** field on `s11_link_t` — identifies the physical link type (`"lte"`, `"5g"`, `"mesh"`, `"lora"`, `"satellite"`). Needed to select the matching `s11_link_model_config_t` for a given link.

## Design notes

- **Delay semantics**: each model returns its own *incremental* `delay_us` only. `network_manager.c` sums across the four calls to get the absolute delay, and stops the chain early on `S11_PACKET_DROPPED`. `s11_process_result_t` is not a running total when returned by an individual model.
- **`link_type` sync**: `s11_link_t.link_type` and `s11_link_model_config_t.link_type` must match in size/name — not compiler-enforced, just a convention. Update both if either changes.

