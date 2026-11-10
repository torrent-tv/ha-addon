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
  the version `package.json` names; there is no `ARG PROXY_VERSION`. The shallow
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
  the proxy's tree also uses. `install-native.sh` then builds and checks the
  native modules (node-datachannel and our `utp-native` fork named by the
  proxy's `overrides`), and the last stage runs `torrent-tv-proxy --help`, so an
  image that cannot load them fails to build. A new proxy reaches the image only
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

The shared rules are in the organization's
[`CONTRIBUTING.md`](https://github.com/torrent-tv/workflows/blob/main/CONTRIBUTING.md);
this repository's own are in
[`openspec/changes/release-with-changesets/design.md`](openspec/changes/release-with-changesets/design.md).
Home Assistant reads the add-on from `main`; there is no `release` branch.

Every pull request adds a changeset in the standard `@changesets/cli` format, in
`torrent_tv_proxy/.changeset/` (`npx changeset` there, or a file
`<branch with / as ->.md`): front matter `"torrent-tv-ha-addon": patch` (or
`minor`; never `major`), then one sentence a user of the add-on understands.
CI (`changeset status --since=origin/main`) refuses a pull request without one;
a change that releases nothing adds an empty one (`changeset add --empty`). A
commit never edits or renames a changeset an earlier commit added. Nothing is
written under `## Unreleased`: `CHANGELOG.md` is written by the release.

Nobody edits the version. After a merge to `main`, the `version` job of
`.github/workflows/main.yml` amends each commit that added a changeset with its
version in `torrent_tv_proxy/package.json`, the lock, its `CHANGELOG.md` entry
and the `version:` line of `config.yaml`, and force-pushes `main` with a lease as
the release App; three changeset commits give three versions. The next run
checks the commit (`scripts/check.sh`: `tools`, `pins`, `config`, `closure`, `image`), the
add-on linter runs, `publish` tags each version (`changeset git-tag`) and creates
its GitHub Release from the `CHANGELOG.md` section, and `rebase-prs` rebases the
open pull requests. The Supervisor follows the force-pushed `main` with a fetch
and `git reset --hard`, so the update appears after the rewrite.

`.github/workflows/dependencies.yml` (daily at 03:27 UTC and on
`workflow_dispatch`) installs the proxy: the proxy's release starts it with
`gh workflow run dependencies.yml -f proxy-version=<version>`, and the daily run
reads npm's latest. It waits until npm serves the version (`npm view
<package>@<version> dist.tarball`, then a HEAD request of the tarball, every
15 s for up to 20 minutes, one log line each round), writes the exact version
into `package.json` and the lock, adds a changeset that carries the proxy's
release notes, and pushes `fix(proxy)` or `feat(proxy): install proxy <version>
#ttv-387` to `main` as the release App. The same run raises what Dependabot
alerts name and applies patch and minor updates (`fix(deps)`/`chore(deps)`
`#ttv-386`), tests each commit with the full checks including the image build,
and pushes those that passed; no pull request is made.

## Updating and checking the add-on on a host

Each Home Assistant host updates its installed add-on separately. From this
machine the Supervisor commands run inside `hassio_cli` over `ssh ha`; the
commands, why the plain `ha` CLI fails over SSH, and how to read the installed
proxy version and the chosen encoder:
[`docs/host-update.md`](docs/host-update.md).
