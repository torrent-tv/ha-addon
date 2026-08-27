ARG BUILD_FROM=ghcr.io/home-assistant/base:latest
FROM ${BUILD_FROM}

RUN apk add --no-cache nodejs npm ffmpeg python3 make g++ tcpdump

# Node must be the LTS line the proxy is developed and tested on. The base
# image supplies whatever Alpine happens to ship, so without this the addon
# would follow it silently onto an untested major — including the odd-numbered
# releases, which never become LTS. Failing the build is the point: a drift
# here is a decision to make, not something to discover from a crash in the
# field.
RUN node -v \
    && MAJOR="$(node -p 'process.versions.node.split(".")[0]')" \
    && if [ "$MAJOR" != "24" ]; then \
         echo "This addon requires Node 24 (LTS); the base image has $(node -v)" >&2; \
         exit 1; \
       fi

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
# The substitution has to reach the WHOLE tree, not the top of it. Installed at
# the proxy's own level only, it left `webtorrent/node_modules/utp-native` in
# place - and Node resolves from the requiring module outward, so webtorrent
# loaded its own copy and the published defect went on killing the process. The
# crash of 2026-08-19 21:03 names that path frame for frame. Nested copies are
# removed after the install, and the build FAILS if any surviving
# `utp_native.node` belongs to a package other than ours: a silent fallback to
# the broken one is what cost a day.
#
# Our build now carries TWO fixes, not one. The second was found on 2026-08-20:
# the UDP read callback parsed a sender address that a FAILED read never
# produced, dereferencing a null pointer on the thread owning the socket.
# Three core dumps in two days, each about three and a half hours into an
# otherwise healthy run. Both fixes are absent from upstream master.
#
# utp-native ships no musl prebuilds (HA base images are Alpine), so without a
# source build uTP is disabled and the proxy reaches peers over TCP only.
# Force-compile it here (toolchain is installed above); best-effort — if the
# build fails the proxy keeps working with TCP-only peers.
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
RUN npm install --global @torrent-tv/proxy --ignore-scripts \
 && cd /usr/local/lib/node_modules/@torrent-tv/proxy/node_modules/node-datachannel \
 && for attempt in 1 2 3 4 5; do \
      npm rebuild --global node-datachannel && break; \
      echo "node-datachannel: build attempt $attempt failed; retrying in 10s" >&2; \
      sleep 10; \
    done \
 && ls build/Release/*.node \
 && (cd /usr/local/lib/node_modules/@torrent-tv/proxy \
     && npm install --ignore-scripts --no-save --no-audit --no-fund \
        "utp-native@npm:@torrent-tv/utp-native@2.5.3-ttv.8") \
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
    done

COPY run.sh /run.sh
COPY poisonmalloc.c /tmp/poisonmalloc.c
RUN chmod a+x /run.sh \
 && gcc -shared -fPIC -O2 -o /usr/lib/poisonmalloc.so /tmp/poisonmalloc.c -ldl -lpthread \
 && ls -lh /usr/lib/poisonmalloc.so

CMD [ "/run.sh" ]
