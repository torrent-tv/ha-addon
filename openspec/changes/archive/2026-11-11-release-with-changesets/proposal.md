# Proposal: Release the add-on with changesets, each commit versioned in place on main

Parent: torrent-tv/meta#336, stage 4 (ha-addon). The plan is the issue's revision 4
(https://github.com/torrent-tv/meta/issues/336#issuecomment-6765817451) and its
revision 5 amendment (https://github.com/torrent-tv/meta/issues/336#issuecomment-6768238527),
with the owner's answers of 2026-11-10. infra (torrent-tv/infra#11) and server
(torrent-tv/server#28) make the same switch; this change merges before proxy's, because the
proxy's release starts `dependencies.yml` here by `workflow_dispatch`, which reaches only a
workflow that is on the default branch.

## Problem

- The add-on version is written into `main` by the release job (`release-prepare` and
  `release-finish` of the shared repository), and the pending changelog is one shared
  `## Unreleased` section. Both are what torrent-tv/meta#336 set out to remove.
- The proxy version is `ARG PROXY_VERSION` in the Dockerfile, edited by the proxy's release
  through the shared action `addon-install-proxy`, which also writes the changelog entry.
- Every Home Assistant build resolves the proxy's dependency tree afresh from the proxy's
  version ranges, because no lock file reaches the image.

## Why

The owner decided (2026-11-10) that every repository releases from changesets, that
`package.json` holds the version, that each commit that adds a changeset gets its own
version amended into it on `main`, and that dependency updates, the proxy install included,
come from a scheduled workflow with no pull request.

## What Changes

- `torrent_tv_proxy/package.json` (`torrent-tv-ha-addon`, private, version `0.100.88`), its
  lock file, `.npmrc` (`save-exact=true`, `install-strategy=shallow`) and
  `.changeset/config.json` with `privatePackages: { version: true, tag: true }`. It names
  `@torrent-tv/proxy` as an exact dependency and `@changesets/cli` 3.0.3,
  `npm-check-updates` 23.1.1 and `semver-tool` (an alias of `semver` 7.8.6, so that npm does not
  share it with the proxy's tree, see design.md Fact 3) as exact devDependencies.
- The Dockerfile installs the proxy with `npm ci --omit=dev --ignore-scripts` from that
  package and lock; `ARG PROXY_VERSION` is gone.
- `main.yml`: the new first job `version` (design.md, "The version job") also writes
  `config.yaml`'s version; `plan`, a matrix of `checks` and `build`, `lint`, `publish` (tag and
  GitHub Release), `rebase-prs`.
- `dependencies.yml`: installs the newest proxy once npm serves it, raises Dependabot alerts,
  applies minor updates, tests each commit, pushes by the release App from a job that runs no
  package code.
- `scripts/check.sh` holds the checks `main.yml` and `dependencies.yml` both run.
- The repository no longer calls any action of `torrent-tv/workflows`, and `AGENTS.md`,
  `README.md` and `docs/host-update.md` describe the new flow.

## Capabilities

### New Capabilities

- `release`: how a commit on `main` becomes a version, a tag and a GitHub Release.
- `dependency-updates`: how the proxy, Dependabot alerts and minor updates reach `main`
  without a pull request.

### Modified Capabilities

- `add-on image`: the proxy comes from `package.json` and its lock, not from an `ARG`.

## Impact

- The release App `torrent-tv-release` rewrites and force-pushes `main`. Its secret
  `RELEASE_APP_PRIVATE_KEY` and variable `RELEASE_APP_CLIENT_ID` are on this repository's
  `production` environment, and the App has `actions: write` and `workflows: write`.
- Home Assistant follows the force-pushed `main` (`supervisor/store/git.py`: fetch, then
  `git reset --hard origin/<branch>` when the tip differs).
- The add-on's image no longer resolves the proxy's dependencies at build time: the lock
  pins the whole tree, and only `dependencies.yml` changes it.
- Node 24.22.2 (`.node-version`) and npm 11.19.1 (`packageManager`) are exact.
