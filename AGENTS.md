Read `../AGENTS.md` first. It holds the project-wide rules (a link to `meta/AGENTS.md` made by meta's setup script; a task worktree sees the same link). This file adds only what is specific to this repository.

# ha-addon — the proxy as a Home Assistant add-on

Packages and runs `@torrent-tv/proxy` as a Home Assistant add-on. This is the
only place for Home Assistant concerns; the proxy itself stays
deployment-agnostic. Slug `torrent_tv_proxy`; Home Assistant shows it as
`b34a1737_torrent_tv_proxy`.

## How it works

- `torrent_tv_proxy/Dockerfile` installs the exact proxy version named by
  `ARG PROXY_VERSION` from npm. The proxy release sets that pin and the add-on
  changelog; changing `config.yaml`'s version alone does not change which proxy
  is installed. The installed `utp-native` build is read from the proxy's own
  `overrides`, and the build fails if any other build survives.
- `torrent_tv_proxy/run.sh` reads the options with bashio and starts the proxy
  with `--server-url`, host and port, and `--ffmpeg-bin "$(command -v ffmpeg)"`
  (the system ffmpeg from apk, so `ffmpeg-static` is not needed).
- `torrent_tv_proxy/config.yaml` — add-on metadata and options. `video: true`
  maps the video devices that exist (`/dev/dri`, `/dev/video*`) so the proxy
  can use a hardware encoder; the proxy tests and falls back to software, so
  this is safe on any host.

## Gotchas

- **Install with lifecycle scripts off.** `ip-set` (through webtorrent) runs
  `npx only-allow pnpm` in `preinstall`, which aborts a plain `npm install`. The
  Dockerfile installs with `--ignore-scripts`, then rebuilds `node-datachannel`
  (retried, and checked: without it there is no WebRTC) and `utp-native` from
  source. Keep this when you touch the Dockerfile.
- **LF line endings** in `run.sh` and the Dockerfile; CRLF breaks the bashio
  shebang and Docker line continuations. CI's line-ending check refuses it.

## Release path

The shared flow, including how a proxy release reaches this repository, is in
the organization's
[`CONTRIBUTING.md`](https://github.com/torrent-tv/.github/blob/main/CONTRIBUTING.md);
here, entries go in `torrent_tv_proxy/CHANGELOG.md` under `## Unreleased` and
the version file is `torrent_tv_proxy/config.yaml`. `.github/workflows/main.yml`
runs the shared checks, checks that npm serves the pinned `PROXY_VERSION`, runs
the add-on linter, builds the image and releases the add-on.

## Updating and checking the add-on on a host

Each Home Assistant host updates its installed add-on separately. From this
machine the Supervisor commands run inside `hassio_cli` over `ssh ha`; the
commands, why the plain `ha` CLI fails over SSH, and how to read the installed
proxy version and the chosen encoder:
[`docs/host-update.md`](docs/host-update.md).
