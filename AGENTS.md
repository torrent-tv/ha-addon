Read `../AGENTS.md` first. It holds the project-wide rules (a link to `meta/AGENTS.md` made by meta's setup script; a task worktree sees the same link). This file adds only what is specific to this repository.

# ha-addon — the proxy as a Home Assistant add-on

Packages and runs `@torrent-tv/proxy` as a Home Assistant add-on. What belongs
here rather than in the proxy:
[proxy `AGENTS.md`, Deployment-agnostic](https://github.com/torrent-tv/proxy/blob/main/AGENTS.md#deployment-agnostic).
Slug `torrent_tv_proxy`; Home Assistant shows it as `b34a1737_torrent_tv_proxy`.

## How it works

- `torrent_tv_proxy/package.json` is the add-on's npm package, private and never
  published. It holds the add-on version (the single edited value; `config.yaml`
  is a copy that the release writes), the exact version of `@torrent-tv/proxy`
  and the exact versions of the tools that release and update it.
  `package-lock.json` pins the whole proxy tree, and `.npmrc` sets
  `save-exact=true` and `install-strategy=shallow`.
- `torrent_tv_proxy/Dockerfile` copies those three files, runs
  `npm ci --omit=dev --ignore-scripts` and fails when the installed proxy is not
  the version `package.json` names; the image takes no proxy version argument. The shallow
  strategy puts the add-on's direct dependencies at the top of `node_modules/`
  and flattens what the proxy needs into the proxy's own `node_modules/`: the
  layout of a global install, which the proxy's `scripts/install-native.sh`
  expects. The final stage copies only the proxy's directory, so that directory
  must hold its whole tree. A package the add-on also depends on directly is
  shared at the top instead (the add-on's `semver` did that to `fastify` and
  `node-abi`), which is why the add-on names its semver tool by an alias
  (`"semver-tool": "npm:semver@..."`). `self-contained.cjs` reads the lock and
  fails the build, before anything is installed, when a package of the proxy's
  tree resolves outside the proxy's directory; `scripts/check.sh closure` runs
  it in CI and in `dependencies.yml`. Never add a direct dependency whose name
  the proxy's tree also uses. `install-native.sh` then checks that
  `node-datachannel` loads (its binary is a per-platform npm package that
  `npm ci` installs; nothing is built or fetched for it) and builds `utp-native`,
  our fork named by the proxy's `overrides`, and the last stage runs
  `torrent-tv-proxy --help`, so an image that cannot load them fails to build. A new proxy reaches the image only
  through `package.json` and the lock, written by `dependencies.yml`.
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

Changesets, versions, the release and the daily dependency update are shared by
every repository and are written once, in the organization's
[`CONTRIBUTING.md`](https://github.com/torrent-tv/workflows/blob/main/CONTRIBUTING.md#versions-and-releases)
("Versions and releases", "How a release reaches its users", "Dependencies").
What is specific to the add-on, and its OpenSpec change:
[`openspec/changes/archive/2026-11-11-release-with-changesets/design.md`](openspec/changes/archive/2026-11-11-release-with-changesets/design.md).

- Home Assistant reads the add-on from `main`; there is no `release` branch. The
  Supervisor follows the force-pushed `main` with a fetch and `git reset --hard`,
  so the update appears after the rewrite.
- Changesets live in `torrent_tv_proxy/.changeset/` (`npx changeset` there), with
  front matter `"torrent-tv-ha-addon"`.
- The `version` job writes the version into `torrent_tv_proxy/package.json`, the
  lock, the `CHANGELOG.md` entry and the `version:` line of `config.yaml`. The
  checks of a commit are `scripts/check.sh`: `tools`, `pins`, `config`, `closure`,
  `image`, and the add-on linter runs. A pull request is also checked by `copies`,
  `changeset-format` and `changesets-selftest` of the same script. Every commit is also checked by
  `workflow-pins` (every action, image, Node version and npm or docker install in a workflow is an exact
  version), `workflow-tokens` (the dependency job `update` gives no token to package code, and each job
  that pushes holds the App token with the permissions it needs, listed in `WORKFLOW_PUSHES`; it reads the
  workflows with yq 4.54.1), `commit-msg-bot` (the dependency job runs the commit-msg hook on each commit
  it prepares), `workflow-refs` (the checkouts of the dependency job `update` are at `main`, so a run
queued behind another starts from the tip of `main`), `workflow-selftest` (all four on bad input).
- `.github/workflows/dependencies.yml` runs daily at 03:27 UTC and on
  `workflow_dispatch` (the proxy's release starts it with `proxy-version=<version>`).
  Before it installs a proxy version it waits until npm serves it
  (`npm view <package>@<version> dist.tarball`, then a HEAD request of the
  tarball, every 15 s for up to 20 minutes, one log line each round). The rest of
  its flow is in the shared sections above.

## Updating and checking the add-on on a host

Each Home Assistant host updates its installed add-on separately. From this
machine the Supervisor commands run inside `hassio_cli` over `ssh ha`; the
commands, why the plain `ha` CLI fails over SSH, and how to read the installed
proxy version and the chosen encoder:
[`docs/host-update.md`](docs/host-update.md).
