# syntax=docker/dockerfile:1.7

# ARG before any FROM makes it available to all FROM instructions.
ARG BUILD_FROM=ghcr.io/home-assistant/base:latest

# Stage 1 – grab a known-good Node.js 24 binary tree from the official image.
FROM node:24-alpine AS node_base

# Stage 2 – HA base image (Alpine-based; version controlled by the addon framework).
FROM ${BUILD_FROM}

# Overlay Node.js 24 from the official image instead of whatever
# version `apk add nodejs` would give us on this Alpine release.
COPY --from=node_base /usr/local/bin/node          /usr/local/bin/node
COPY --from=node_base /usr/local/lib/node_modules  /usr/local/lib/node_modules
COPY --from=node_base /usr/local/bin/npm           /usr/local/bin/npm
COPY --from=node_base /usr/local/bin/npx           /usr/local/bin/npx

# ffmpeg is needed for optional HLS audio transcode mode.
RUN apk add --no-cache ffmpeg

# Install the latest proxy package from npm.
RUN npm install --global @torrent-tv/proxy

COPY run.sh /run.sh
RUN chmod a+x /run.sh

CMD [ "/run.sh" ]
