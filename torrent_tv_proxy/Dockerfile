ARG BUILD_FROM=ghcr.io/home-assistant/base:latest

# Two stages. The first installs the proxy and compiles its native modules; the
# second is what runs, and receives only the proxy and the one library built
# for it. The compiler, python, npm, its cache and node-gyp's headers stay in
# the first: measured on the HA host on 2026-10-05, they, gdb and a
# devDependency the single stage kept made up about 480 MB of a 901 MB image.
FROM ${BUILD_FROM} AS build

RUN apk add --no-cache nodejs npm python3 make g++

# BUILD_VERSION is injected automatically by the HA supervisor from config.yaml,
# and the install below MENTIONS IT so that the layer is invalidated whenever
# the addon version changes. Keep PROXY_VERSION aligned with the version
# recorded in CHANGELOG.md; pinning avoids a build
# resolving the previous npm `latest` tag while a release is still propagating.
#
# Declaring the ARG is not enough, and that is what kept shipping an old proxy
# under a new addon version: Docker keys a RUN layer on the command text, and a
# build argument enters that key only where it is actually referenced. Addon
# 0.74.6 and 0.75.0 both said they carried a proxy they did not — 0.75.0 was
# built with this layer cached and installed 2.83.8 under a version claiming
# 2.84.0.
ARG BUILD_VERSION
ARG PROXY_VERSION=2.107.77
# NOTE: ip-set (a transitive dependency of webtorrent) ships a hostile
# `preinstall: npx only-allow pnpm` script that aborts any plain `npm install`
# (exit code 254).  Install with lifecycle scripts disabled to bypass it; the
# script below builds the native modules that genuinely need theirs.
# The proxy is started with --ffmpeg-bin pointing at the system ffmpeg
# (installed in the final stage via apk), so ffmpeg-static's bundled binary is
# not required.
# The native modules (node-datachannel, our utp-native) and the Node floor are
# built and checked by the proxy's own scripts/install-native.sh, which ships in
# the package; see that script for what it does and why.
RUN echo "building for addon ${BUILD_VERSION} with proxy ${PROXY_VERSION}" \
 && npm install --global "@torrent-tv/proxy@${PROXY_VERSION}" --ignore-scripts \
 && sh /usr/local/lib/node_modules/@torrent-tv/proxy/scripts/install-native.sh /usr/local/lib/node_modules/@torrent-tv/proxy

COPY poisonmalloc.c /tmp/poisonmalloc.c
RUN gcc -shared -fPIC -O2 -o /usr/lib/poisonmalloc.so /tmp/poisonmalloc.c -ldl -lpthread \
 && ls -lh /usr/lib/poisonmalloc.so

FROM ${BUILD_FROM}

# What the proxy runs with: node, ffmpeg, and tcpdump for the packet witness,
# which is on by default. gdb is not here: only `--usrsctp-state` uses it, and
# run.sh never passes that; a core dump can still be read after `apk add gdb`
# in the running container.
RUN apk add --no-cache nodejs ffmpeg tcpdump

COPY --from=build /usr/local/lib/node_modules/@torrent-tv/proxy /usr/local/lib/node_modules/@torrent-tv/proxy
COPY --from=build /usr/lib/poisonmalloc.so /usr/lib/poisonmalloc.so
COPY run.sh /run.sh
# `--help` loads every module of the main thread, node-datachannel's native
# binding included, so a node here that cannot run what the first stage built
# fails the build instead of the first start.
RUN ln -s ../lib/node_modules/@torrent-tv/proxy/bin/cli.js /usr/local/bin/torrent-tv-proxy \
 && chmod a+x /run.sh \
 && torrent-tv-proxy --help > /dev/null

CMD [ "/run.sh" ]
