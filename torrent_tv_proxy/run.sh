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
)

if [ -n "${TOKEN}" ]; then
  ARGS+=(--token "${TOKEN}")
fi

if bashio::var.has_value "${SEGMENT_FORMAT}"; then
  bashio::log.info "HLS segment format: ${SEGMENT_FORMAT}"
  ARGS+=(--segment-format "${SEGMENT_FORMAT}")
fi

exec torrent-tv-proxy "${ARGS[@]}"
