# ha-addon — Home Assistant addon for the proxy

Packages and runs `@torrent-tv/proxy` as a Home Assistant addon. See the parent
`../AGENTS.md` for the overall architecture and release process. This is the
ONLY place Home-Assistant-specific concerns belong (the proxy itself stays
deployment-agnostic).

Addon slug: `torrent_tv_proxy` (HA UI shows `6630818b_torrent_tv_proxy`; repository
`https://github.com/torrent-tv/ha-addon#release`; it was `b34a1737_…` until the
meta#336 migration).

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

## Commits

Every commit header follows Conventional Commits (`<type>(<scope>)!: <subject>`,
types `feat fix perf refactor docs test build ci chore style revert`); CI refuses
a pushed commit that does not. Enable the local check once per clone:
`git config core.hooksPath .githooks`. Rules: `torrent-tv/.github` CONTRIBUTING.md.

## Changesets and release

A pull request describes its change in `.changeset/<branch>.md` (the branch name,
`/` replaced by `-`): one or two bullets in the `- **New**:`/`- **Fix**:`/
`- **Chore**:` format that tell a viewer what changed. CI (`check-changesets`)
refuses a pull request without one. Never write a version heading and never edit
the `config.yaml` version: the release does both.

What `main` holds: `torrent_tv_proxy/config.yaml` `version` is the last release
made from `main` (`0.100.64`) and stays; `ARG PROXY_VERSION` in the Dockerfile is
the least proxy any release installs (a pull request raises it only to require a
newer proxy); `CHANGELOG.md` is frozen with a pointer line. A release never
writes `main`.

GitHub Actions releases. A push to `main` runs `.github/workflows/main.yml`: checks
(commit headers, line endings, workflows, changeset on pull requests), the add-on
linter, then the release job. It plans the version from the newest `v*` tag and the
commits after it (`feat` -> minor; `fix`/`perf`/`revert` -> patch) and the notes from
the changesets, builds one commit on top of the `main` commit (version in
`config.yaml`, proxy in the Dockerfile, `CHANGELOG.md` = new section + the previous
release's changelog), builds the image and checks the proxy in it, tags it
`v<version>`, creates the GitHub Release and force-pushes the tag's commit as branch
`release`. Home Assistant reads the add-on from
`https://github.com/torrent-tv/ha-addon#release`. Never write branch `release` by
hand. Re-runs are safe: a run whose tag exists only moves the branch.

A proxy release arrives as the `repository_dispatch` event `proxy-release`
(`torrent-tv/.github` `actions/addon-dispatch-proxy`, payload
`{version, bump, tag, notes}`) and releases `main` with that proxy, at least a patch.
`workflow_dispatch` `bump` is a least step. The order proxy -> add-on is kept by the
dispatch, which waits for npm.

## Gotchas

- **ip-set / pnpm**: a transitive proxy dependency ships
  `preinstall: npx only-allow pnpm`, which aborts a plain `npm install`. The
  Dockerfile installs with `--ignore-scripts` and then
  `npm rebuild --global node-datachannel` (the one native module that needs its
  install script). Keep this if you touch the Dockerfile.
- **LF line endings** are mandatory for `run.sh` and `Dockerfile` (CRLF breaks
  the bashio shebang / Docker line continuations). Editing on Windows can
  reintroduce CRLF — strip it (`sed -i 's/\r$//'`).
- **Release order**: the proxy is published to npm before the add-on installs it;
  the dispatch keeps this order (see "Changesets and release") and the release
  refuses a proxy that npm does not serve.
- **Triggering the update remotely over SSH (`ssh ha`, verified 2026-08-01):**
  the standalone `/usr/bin/ha` CLI on the host is UNauthenticated by default —
  `ha store reload`/`ha apps update ...` fail with `unauthorized: missing or
  invalid API token` (no `~/.homeassistant.yaml`, no token env var for the SSH
  user). Supervisor already runs a properly-token-wired `hassio_cli` container
  for exactly this — run the same commands INSIDE it instead:
  `sudo docker exec hassio_cli ha store reload` then
  `sudo docker exec hassio_cli ha apps update 6630818b_torrent_tv_proxy`.
  The SSH user (`silentimp`) is in `wheel` with passwordless `sudo` but is NOT
  in the `docker` group, so plain `docker ...` fails with a socket permission
  error — always prefix `sudo`. If update returns `Error: Another job is
  running for job group app_6630818b_torrent_tv_proxy`, an update is already
  in flight (check with `sudo docker logs app_builder_6630818b_torrent_tv_proxy
  --tail 40` — it's a real multi-stage Docker build, ~90s: apk/npm install,
  native module rebuild, image export); just poll until the builder container
  disappears rather than re-issuing the update. Verify success with
  `sudo docker ps --filter name=app_6630818b_torrent_tv_proxy` (image tag =
  the new addon version) and
  `sudo docker exec app_6630818b_torrent_tv_proxy node -e "fetch('http://127.0.0.1:9090/healthz').then(r=>r.text()).then(console.log)"`
  (the runtime stage has no npm; this reads the proxy health endpoint, not the
  addon version).
- Refresh the store with `ha store reload` (NOT `ha addons reload`) before the
  UI shows the update; then `ha apps update 6630818b_torrent_tv_proxy` or the UI
  Update button (reload the page if it looks stale). From the UI/HA-local
  shell this works directly (a real Supervisor token is present there); the
  SSH detour above is only needed from an external session like this one.
- Verify the running encoder in the addon log: `hwaccel: using hardware encoder
  …` or `… failed the HLS keyframe-alignment test; skipping` → `… using
  software libx264`.
