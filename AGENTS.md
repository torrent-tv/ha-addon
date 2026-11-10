Read `../AGENTS.md` first. It holds the project-wide rules (a link to `meta/AGENTS.md` made by meta's setup script; a task worktree sees the same link). This file adds only what is specific to this repository.

# ha-addon — the proxy as a Home Assistant add-on

Packages and runs `@torrent-tv/proxy` as a Home Assistant add-on. What belongs
here rather than in the proxy:
[proxy `AGENTS.md`, Deployment-agnostic](https://github.com/torrent-tv/proxy/blob/main/AGENTS.md#deployment-agnostic).
Slug `torrent_tv_proxy`; Home Assistant shows it as `b34a1737_torrent_tv_proxy`.

## How it works

- `torrent_tv_proxy/Dockerfile` installs the exact proxy version named by
  `ARG PROXY_VERSION` from npm. The proxy release sets that pin and the add-on
  changelog; changing `config.yaml`'s version alone does not change which proxy
  is installed. The proxy's own `scripts/install-native.sh`, shipped in the
  package, then builds and checks its native modules (node-datachannel and our
  `utp-native` fork named by the proxy's `overrides`), and the last stage runs
  `torrent-tv-proxy --help`, so an image that cannot load them fails to build.
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
  Dockerfile installs with `--ignore-scripts`, and the native modules that need
  their scripts are built by the proxy's `scripts/install-native.sh`. Keep this
  when you touch the Dockerfile.
- **LF line endings** in `run.sh` and the Dockerfile; CRLF breaks the bashio
  shebang and Docker line continuations. `.gitattributes` stores every text file with LF.

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
