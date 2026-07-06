## 0.2.46

- **Fix**: Actually deliver proxy 2.9.24. The 0.2.45 image was built before 2.9.24 reached the npm registry, so it silently kept proxy 2.9.22 — whose blocking codec-probe request breaks cold-start playback against the current server (the browser expects the fast `pending` + poll protocol from 2.9.24 and times out after 60 s with "Data channel request timed out"). This rebuild installs the published 2.9.24.
- **New**: uTP peer connections. `utp-native` ships no musl prebuilds (HA base images are Alpine), so uTP was disabled on every host and peers were reachable over TCP only. The Dockerfile now compiles it from source at build time (toolchain was already present); best-effort — if the build fails, the proxy keeps working with TCP-only peers.

## 0.2.45

- **New**: Bump to pull proxy 2.9.24 — IPv6-first, cold-start fix, and disk hygiene. (1) IPv6: a second IPv6-capable STUN server so a proxy with global IPv6 gathers a v6 candidate, allowing a direct (NAT-free) connection to v6-native viewers (e.g. on cellular); candidate logs now tag address scope. (2) Cold-start: the codec-probe request returns quickly while the file header downloads (browser polls instead of one blocking request), so a torrent whose peers are still connecting plays once the header arrives instead of failing after 60 s (requires server 0.8.24, already live). (3) Disk hygiene: idle torrents are removed with their on-disk store after a 5-min idle TTL, and orphaned data from a previous hard kill is swept at startup — so downloaded files no longer pile up on the host. No behaviour change for IPv4-only proxies.

## 0.2.44

- **New**: Bump to pull proxy 2.9.23 — symmetric-NAT port prediction for WebRTC. On a symmetric NAT the proxy now offers predicted external ports (base + delta·k) alongside its real srflx candidate, so a viewer can reach it without manual port forwarding when the NAT increments ports predictably. No effect on cone NATs. Diagnostic logging included.

## 0.2.43

- **Fix**: Bump to pull proxy 2.9.22 — fixes a proxy crash on repeat/remote WebRTC sessions (an unguarded WebRTC error took down the whole process and restarted the addon). The WebRTC UDP port is now held by one persistent ICE mux listener for the proxy's lifetime, so reconnects and concurrent viewers share the single fixed port without failing to gather ICE candidates.

## 0.2.42

- **Chore**: Bump to pull proxy 2.9.21 — adds a per-poll `[stats]` log (peers, download speed, file %, header bytes) so a cold-start torrent download stall is visible in the addon log. Diagnostic only.

## 0.2.41

- **Chore**: Bump to pull proxy 2.9.20 — the proxy classifies its home NAT at startup (queries two public STUN servers and compares the external port) and logs whether it is endpoint-independent (cone — WebRTC fixed-port mapping suffices) or symmetric (would need port prediction). Diagnostic; no behaviour change to playback.

## 0.2.40

- **Chore**: Bump to pull proxy 2.9.19 — diagnostics only: the proxy logs successful port-mapping removal on shutdown, and logs full WebRTC ICE candidates, ICE-state transitions, and the selected candidate pair on connect (for verifying remote reachability and root-causing connection failures from the addon log).

## 0.2.39

- **Chore**: Bump to pull proxy 2.9.18 — WebRTC is now pinned to a single, UPnP-mapped UDP port (multiplexed across sessions via ICE UDP mux), so the proxy's WebRTC path is reachable from the internet without manual port forwarding. Declares `9090/udp` in the addon manifest (cosmetic under `host_network: true`, but documents intent).

## 0.2.38

- **Chore**: Bump to pull proxy 2.9.17 — the proxy now reports its UPnP-mapped external endpoint to the server so the server can dial back and verify the proxy is reachable from the internet (paired with server 0.8.22).

## 0.2.37

- **Chore**: Bump to pull proxy 2.9.16 — automatic UPnP/NAT-PMP port mapping at startup (opens TCP 9090 on the home router with an auto-renewed lease, removed on shutdown) so the proxy can be reached without manual port forwarding. Best-effort: a router without UPnP is logged and ignored. Disable with `--no-port-mapping`.

## 0.2.36

- **Chore**: Bump to pull proxy 2.9.15 — torrent data is now removed on graceful shutdown (`destroyStore` on each torrent + client teardown, wired into the proxy's `onClose` hook), so downloaded files no longer accumulate under the temp dir after the addon stops/restarts.

## 0.2.35

- **Chore**: Bump to pull proxy 2.9.14 — stats report header/index download progress so the browser can show the download phase's ETA and % toward the transcode phase.

## 0.2.34

- **Chore**: Bump to pull proxy 2.9.13 — video-copy start fix (0-based timeline, no start hole / audio-video desync) and keyframe-aligned segment boundaries for gap-free seeking on copied video.

## 0.2.33

- **Chore**: Bump to pull proxy 2.9.12 — fixes PTS-gap glitches (video stutter with audio still playing) on start/seek for both re-encoded video (fixed GOP) and copied video (`-copyts`); session log tags the active branch.

## 0.2.32

- **Chore**: Bump to pull proxy 2.9.11 — seek-aware torrent piece prioritization (marks the read position critical), so seeking into an undownloaded region no longer stalls ~15-18 s waiting for data.

## 0.2.31

- **Chore**: Bump to pull proxy 2.9.10 — larger adaptive-preset speed margin (1.3 → 1.8) so the chosen preset keeps playback above realtime under real load (WebTorrent + delivery contention) instead of stalling.

## 0.2.30

- **Chore**: Bump to pull proxy 2.9.8 — faster software transcode (all cores, no upscaling) with an adaptive preset chosen by a startup benchmark (highest quality that still keeps ≥1× realtime), plus a restart-cooldown that stops ffmpeg thrashing between seek positions.

## 0.2.29

- **Chore**: Bump to pull proxy 2.9.7 — playback-plan codec probe retries instead of caching an empty result, fixing black-video on files whose header downloads late (e.g. a later episode in a multi-file torrent).

## 0.2.28

- **Chore**: Bump to pull proxy 2.9.6 — duration probe returns on the container header instead of decoding the whole stream, cutting transcode-session creation from ~9.7 s to ~1 s.

## 0.2.27

- **Chore**: Bump to pull proxy 2.9.5 — segment files read with a 4 MB `highWaterMark` to deliver the body in fewer, larger chunks (cuts first-segment transfer time on busy ARM hosts).

## 0.2.26

- **Chore**: Bump to pull proxy 2.9.4 — temporary `[net-debug]` transfer-timing instrumentation (`fetchMs`/`ttfbMs`/`sendMs`/`chunks`) for diagnosing early-segment latency.

## 0.2.25

- **Chore**: Bump to pull proxy 2.9.3 — WebRTC data-channel binary response frames + send-loop backpressure. Requires server ≥ 0.8.0 (deploy the server before publishing the proxy and bumping this addon).

## 0.2.8

- **Fix**: ICE candidate filtering in proxy 2.5.6 — PNA permission dialog no longer appears when connecting from HTTPS pages. Requires proxy 2.5.6+.
- **Chore**: Bumped addon version to force Docker layer cache invalidation and install the updated proxy package.

## 0.2.7

- **Fix**: Tunnel keepalive — proxy 2.5.5 prevents Cloudflare from dropping the WebSocket tunnel after ~100 s of inactivity. Bump to pull in the fix.

## 0.2.5

- Fix Docker layer caching: addon version now automatically busts the npm install cache on every rebuild, ensuring the latest proxy package is always installed.

## 0.2.4

- Reverted Node.js upgrade complexity; addon now uses the Node.js version bundled with the HA base image.

## 0.2.0

- **New**: WebRTC P2P tunnel architecture — the proxy now opens a persistent WebSocket tunnel to the server on startup. Video streams are delivered directly from Home Assistant to the browser over a WebRTC data channel, with the server acting only as a signalling relay.
- **Removed**: `public_base_url` config option — no longer needed with the tunnel-based architecture.
- **Changed**: Default `server_url` is now `https://webauth.courses`.
- **Fixed**: Startup crash when `public_base_url` was absent from the saved addon configuration.

## 0.1.7

- Previous stable release with HTTP heartbeat-based proxy registration.
