#!/usr/bin/with-contenv bashio
set -euo pipefail

SERVER_URL="$(bashio::config 'server_url')"
TOKEN="$(bashio::config 'token')"
# Persisted by Supervisor across restarts/updates; falls back to the proxy's own
# default when an older config has no such key yet.
SEGMENT_FORMAT="$(bashio::config 'segment_format')"

if [ -z "${SERVER_URL}" ]; then
  bashio::exit.nok "Option 'server_url' is required."
fi

ARGS=(
  --server-url "${SERVER_URL}"
  --host 0.0.0.0
  --port 9090
  # Use the system ffmpeg installed in the image (ffmpeg-static's bundled
  # binary is intentionally not downloaded — see Dockerfile).
  --ffmpeg-bin "$(command -v ffmpeg)"
  # What this host has measured about itself — how long it takes to create a
  # session and to make a first segment — so a viewer arriving after a restart
  # is shown a figure with a measurement behind it. /data is the only directory
  # the Supervisor keeps: everything else lives in the container's writable
  # layer and is discarded when an update rebuilds it.
  --state-dir /data
)

# Node resolves host names through `getaddrinfo` on the libuv thread pool, and
# that pool holds FOUR threads by default. A torrent announces to every tracker
# in its file at once — ten or thirteen of them — so four names are resolved and
# the rest queue; a tracker that no longer exists holds its thread for the full
# ten-second resolver timeout, and every announce behind it blows its own
# fifteen-second deadline. Measured inside this container 2026-08-18: the ten
# trackers of one film took 7.6 s to resolve as a burst and 27-42 ms each with a
# larger pool, and the film — 517 seeders on a tracker that answers in 50 ms —
# spent eleven minutes with zero peers because every announce timed out.
#
# Sized to hold several torrents' announce lists at once, since the pool also
# serves this process's file reads. Idle threads cost memory and nothing else.
export UV_THREADPOOL_SIZE=64

if [ -n "${TOKEN}" ]; then
  ARGS+=(--token "${TOKEN}")
fi

if bashio::var.has_value "${SEGMENT_FORMAT}"; then
  bashio::log.info "HLS segment format: ${SEGMENT_FORMAT}"
  ARGS+=(--segment-format "${SEGMENT_FORMAT}")
fi

exec torrent-tv-proxy "${ARGS[@]}"
