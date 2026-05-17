#!/usr/bin/with-contenv bashio
set -euo pipefail

SERVER_URL="$(bashio::config 'server_url')"
TOKEN="$(bashio::config 'token')"

if [ -z "${SERVER_URL}" ]; then
  bashio::exit.nok "Option 'server_url' is required."
fi

ARGS=(
  --server-url "${SERVER_URL}"
  --host 0.0.0.0
  --port 9090
)

if [ -n "${TOKEN}" ]; then
  ARGS+=(--token "${TOKEN}")
fi

exec torrent-tv-proxy "${ARGS[@]}"
