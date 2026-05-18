ARG BUILD_FROM=ghcr.io/home-assistant/base:latest
FROM ${BUILD_FROM}

RUN apk add --no-cache nodejs npm ffmpeg python3 make g++

# BUILD_VERSION is injected automatically by the HA supervisor from config.yaml.
# Declaring it here ensures the npm install layer is invalidated whenever
# the addon version changes — no need to pin the npm package version manually.
ARG BUILD_VERSION
RUN npm install --global @torrent-tv/proxy

COPY run.sh /run.sh
RUN chmod a+x /run.sh

CMD [ "/run.sh" ]
