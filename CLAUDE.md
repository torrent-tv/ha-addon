# ha-addon — Home Assistant addon for the proxy

Packages and runs `@torrent-tv/proxy` as a Home Assistant addon. See the parent
`../CLAUDE.md` for the overall architecture and release process. This is the
ONLY place Home-Assistant-specific concerns belong (the proxy itself stays
deployment-agnostic).

Addon slug: `torrent_tv_proxy` (HA UI shows `b34a1737_torrent_tv_proxy`).

## How it works

- `torrent_tv_proxy/Dockerfile` installs the proxy globally from npm:
  `npm install --global @torrent-tv/proxy`. Bumping `config.yaml` `version`
  (via the `ARG BUILD_VERSION` cache-bust) forces a rebuild that pulls the
  latest published proxy.
- `torrent_tv_proxy/run.sh` launches it with `--server-url`, host/port, and
  `--ffmpeg-bin "$(command -v ffmpeg)"` — the system ffmpeg installed via apk
  (so ffmpeg-static's binary is not needed).
- `torrent_tv_proxy/config.yaml` — addon metadata. `video: true` maps all
  available video devices (GPU `/dev/dri`, `/dev/video*`) into the container so
  the proxy can use hardware transcoding when present; host-agnostic (only
  existing devices are mapped). The proxy auto-detects and falls back to
  software, so this is safe on any host.

## Changelog

Every addon release must be recorded in `torrent_tv_proxy/CHANGELOG.md` — add an
entry under a new `## <version>` heading at the top (version from
`torrent_tv_proxy/config.yaml`), following the existing
`- **New**/**Fix**/**Chore**:` format. Bump and add an entry whenever the proxy
changes, since the addon must be re-released to pull it. See the parent
`../CLAUDE.md`.

## Gotchas

- **ip-set / pnpm**: a transitive proxy dependency ships
  `preinstall: npx only-allow pnpm`, which aborts a plain `npm install`. The
  Dockerfile installs with `--ignore-scripts` and then
  `npm rebuild --global node-datachannel` (the one native module that needs its
  install script). Keep this if you touch the Dockerfile.
- **LF line endings** are mandatory for `run.sh` and `Dockerfile` (CRLF breaks
  the bashio shebang / Docker line continuations). Editing on Windows can
  reintroduce CRLF — strip it (`sed -i 's/\r$//'`).
- **Release order**: publish the proxy to npm FIRST (`npm run patch` in
  `../proxy`), THEN bump this `config.yaml` version + push + update the addon.
  Otherwise the rebuild reinstalls the OLD proxy.
- Refresh the store with `ha store reload` (NOT `ha addons reload`) before the
  UI shows the update; then `ha apps update b34a1737_torrent_tv_proxy` or the UI
  Update button (reload the page if it looks stale).
- Verify the running encoder in the addon log: `hwaccel: using hardware encoder
  …` or `… failed the HLS keyframe-alignment test; skipping` → `… using
  software libx264`.
