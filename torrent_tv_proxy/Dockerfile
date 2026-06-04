ARG BUILD_FROM=ghcr.io/home-assistant/base:latest
FROM ${BUILD_FROM}

RUN apk add --no-cache nodejs npm ffmpeg python3 make g++

# BUILD_VERSION is injected automatically by the HA supervisor from config.yaml.
# Declaring it here ensures the npm install layer is invalidated whenever
# the addon version changes — no need to pin the npm package version manually.
ARG BUILD_VERSION
# NOTE: ip-set (a transitive dependency of webtorrent) ships a hostile
# `preinstall: npx only-allow pnpm` script that aborts any plain `npm install`
# (exit code 254).  Install with lifecycle scripts disabled to bypass it, then
# rebuild only the native addon that genuinely needs its install script.
# The proxy is started with --ffmpeg-bin pointing at the system ffmpeg
# (installed above via apk), so ffmpeg-static's bundled binary is not required.
RUN npm install --global @torrent-tv/proxy --ignore-scripts \
 && npm rebuild --global node-datachannel

COPY run.sh /run.sh
RUN chmod a+x /run.sh

CMD [ "/run.sh" ]
