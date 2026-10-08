ARG BUILD_FROM=ghcr.io/home-assistant/base:latest

# Two stages. The first installs the proxy and compiles its native modules; the
# second is what runs, and receives only the proxy and the one library built
# for it. The compiler, python, npm, its cache and node-gyp's headers stay in
# the first: measured on the HA host on 2026-10-05, they, gdb and a
# devDependency the single stage kept made up about 480 MB of a 901 MB image.
FROM ${BUILD_FROM} AS build

RUN apk add --no-cache nodejs npm python3 make g++

# Node must be at least the line the proxy is developed on. The base image
# supplies whatever Alpine happens to ship, so without this the addon could
# follow it silently onto a major OLDER than the code needs. Failing the build
# is the point: a drift here is a decision to make, not something to discover
# from a crash in the field.
#
# A FLOOR, NOT A PIN, since 2026-09-20. It named one major exactly, which also
# forbade every major ABOVE it — a prohibition nothing had measured. The
# package's own `engines` carried the same bound and has been opened the same
# way.
RUN node -v \
    && MAJOR="$(node -p 'process.versions.node.split(".")[0]')" \
    && if [ "$MAJOR" -lt 24 ]; then \
         echo "This addon requires Node 24 or newer; the base image has $(node -v)" >&2; \
         exit 1; \
       fi

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
ARG PROXY_VERSION=2.95.14
# NOTE: ip-set (a transitive dependency of webtorrent) ships a hostile
# `preinstall: npx only-allow pnpm` script that aborts any plain `npm install`
# (exit code 254).  Install with lifecycle scripts disabled to bypass it, then
# rebuild only the native addon that genuinely needs its install script.
# The proxy is started with --ffmpeg-bin pointing at the system ffmpeg
# (installed in the final stage via apk), so ffmpeg-static's bundled binary is
# not required.
# The substitution has to reach the WHOLE tree, not the top of it. Installed at
# the proxy's own level only, it left `webtorrent/node_modules/utp-native` in
# place - and Node resolves from the requiring module outward, so webtorrent
# loaded its own copy and the published defect went on killing the process. The
# crash of 2026-08-19 21:03 names that path frame for frame. Nested copies are
# removed after the install, and the build FAILS if any surviving
# `utp_native.node` belongs to a package other than ours: a silent fallback to
# the broken one is what cost a day. That install runs inside the proxy's own
# directory, where npm would also install its devDependencies — 55.5 MB of
# @biomejs in every image until `--omit=dev` was added.
#
# Our build now carries TWO fixes, not one. The second was found on 2026-08-20:
# the UDP read callback parsed a sender address that a FAILED read never
# produced, dereferencing a null pointer on the thread owning the socket.
# Three core dumps in two days, each about three and a half hours into an
# otherwise healthy run. Both fixes are absent from upstream master.
#
# WHICH build is read from the proxy being installed, from its `overrides`, and
# from nowhere else. It used to be written here as well, and the two had to move
# together: a proxy pinning one version and this line installing another makes
# npm refuse with EOVERRIDE, which is what failed addon 0.36.0. With one place
# to say it, a proxy release that moves the version brings the build with it.
#
# utp-native ships no musl prebuilds (HA base images are Alpine), so without a
# source build uTP is disabled and the proxy reaches peers over TCP only.
# Force-compile it here (toolchain is installed above); best-effort — if the
# build fails the proxy keeps working with TCP-only peers. The compiler's
# intermediate objects are removed once the module is checked.
# node-datachannel's own install script fetches a prebuilt binary from GitHub
# and, if that ONE request fails, falls back to building from source — a
# fallback that is itself broken in `prebuild` 13 ("expected first argument to
# be an array"), so a momentary network hiccup ends the whole build with an
# error about the wrong thing entirely. That is exactly what happened on
# 2026-08-12: `prebuild-install warn install socket hang up`, and immediately
# afterwards the same file downloaded from this host in 0.6 s. The download is
# therefore retried, and the result is CHECKED rather than assumed — an addon
# that starts without this module has no WebRTC at all, which is the whole
# transport.
RUN echo "building for addon ${BUILD_VERSION} with proxy ${PROXY_VERSION}" \
 && npm install --global "@torrent-tv/proxy@${PROXY_VERSION}" --ignore-scripts \
 && cd /usr/local/lib/node_modules/@torrent-tv/proxy/node_modules/node-datachannel \
 && for attempt in 1 2 3 4 5; do \
      npm rebuild --global node-datachannel && break; \
      echo "node-datachannel: build attempt $attempt failed; retrying in 10s" >&2; \
      sleep 10; \
    done \
 && ls build/Release/*.node \
 && (cd /usr/local/lib/node_modules/@torrent-tv/proxy \
     && utp_spec="$(node -p "require('./package.json').overrides['utp-native']")" \
     && case "$utp_spec" in \
          npm:@torrent-tv/utp-native@*) echo "utp-native: the proxy pins $utp_spec" ;; \
          *) echo "FATAL: the proxy pins no utp-native build of ours: $utp_spec" >&2; exit 1 ;; \
        esac \
     && npm install --omit=dev --ignore-scripts --no-save --no-audit --no-fund \
        "utp-native@${utp_spec}") \
 && find /usr/local/lib/node_modules/@torrent-tv/proxy/node_modules -mindepth 3 \
      -type d -name utp-native -prune -exec rm -rf {} + \
 && (npm_config_build_from_source=true npm rebuild --global utp-native \
     || echo "WARNING: utp-native build failed; peer connections stay TCP-only") \
 && for module in $(find /usr/local/lib/node_modules/@torrent-tv/proxy/node_modules \
      -path '*/build/Release/utp_native.node' -not -path '*obj.target*'); do \
      pkg="$(dirname "$(dirname "$(dirname "$module")")")/package.json"; \
      grep -q '"name": "@torrent-tv/utp-native"' "$pkg" \
        || { echo "FATAL: $module is not our build" >&2; exit 1; }; \
      echo "utp-native: our build at $module"; \
    done \
 && find /usr/local/lib/node_modules/@torrent-tv/proxy/node_modules -path '*/build/Release/*' \
      \( -name obj.target -o -name .deps \) -prune -exec rm -rf {} +

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
