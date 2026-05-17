ARG BUILD_FROM=ghcr.io/home-assistant/base:latest
FROM ${BUILD_FROM}

RUN apk add --no-cache nodejs npm ffmpeg

# Install the latest proxy package from npm.
RUN npm install --global @torrent-tv/proxy

COPY run.sh /run.sh
RUN chmod a+x /run.sh

CMD [ "/run.sh" ]
