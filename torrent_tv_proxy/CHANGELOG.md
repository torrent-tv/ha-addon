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
