# Proposal: Release the add-on to a generated `release` branch

Parent: torrent-tv/meta#336, step 3 (ha-addon). Plan point 6, Acceptance 3.
Step 1 (`check-changesets`, `release-plan`, `release-tag`, `release-from-tags.mjs`
with `--last-tag newest`, `--tagged-head ignore`, `--extra-notes`) is merged in
`torrent-tv/.github` (`bc381ae`). Branch `ci/ttv-336-release-branch`, from
`origin/main` `bb83b2d` (`chore(release): 0.100.64`, tag `v0.100.64`).

## Problem

Every add-on release commits `chore(release): <version>` to `main`
(`release-prepare` + `release-finish`, `.github/workflows/main.yml:107-120`), and
every proxy release pushes `fix(proxy)`/`feat(proxy): install proxy <v>` to `main`
(`torrent-tv/.github` `actions/addon-install-proxy:50-90`). Home Assistant builds
the add-on on the host from the repository (`config.yaml` has no `image:`), so the
branch it reads must carry `config.yaml` `version`, the Dockerfile's
`ARG PROXY_VERSION` and `CHANGELOG.md` (shown in the update dialog).

## Why

Release state written back into `main` is the race that meta#336 removes. Home
Assistant can read a repository from a branch (`url#branch`, verified on the
owner's Supervisor 2026.10.5, meta#336 first comment), so the release can live on
a branch of its own that `main` never sees.

## What Changes

- A release builds one commit **R** on top of the main commit **M** it releases.
  R changes exactly three files: `torrent_tv_proxy/config.yaml` (version),
  `torrent_tv_proxy/Dockerfile` (`ARG PROXY_VERSION`) and
  `torrent_tv_proxy/CHANGELOG.md` (the previous release's changelog under a new
  `## <version>` section from the notes). R is tagged `v<version>`, a GitHub
  Release is created, and branch `release` is force-pushed to R (with a lease on
  the commit it replaces, only from a commit an older release tag names).
- `main` is never written by a release. Its `config.yaml` version and
  `ARG PROXY_VERSION` stay the values of the last release made from `main`
  (`0.100.64`, `2.107.64`); `ARG PROXY_VERSION` on `main` is the **least** proxy a
  release installs. `CHANGELOG.md` on `main` is frozen with a pointer line.
- Triggers of `.github/workflows/main.yml`: `push` to `main` (release the add-on
  change with the newest release's proxy), `repository_dispatch` `proxy-release`
  (release `main` with the dispatched proxy; nothing when the newest release
  already installs it or a newer one), `workflow_dispatch` (least step).
  Concurrency `release`, no cancel.
- Pull requests add `.changeset/<branch>.md` (`check-changesets` replaces
  `check-changelog`); the PR image build uses the proxy the next release installs.
- `torrent-tv/.github`: new script `scripts/addon-release.mjs` (subcommands
  `proxy`, `commit`, `branch`, `installs`) with tests, new actions
  `addon-release-plan`, `addon-release-branch`, `addon-dispatch-proxy`; the wait of
  `addon-install-proxy` reads the newest tag of the repository instead of the
  newest tag on `main`, so a proxy release during the switch still ends green.
- `torrent-tv/proxy`: `addon-install-proxy` is replaced by `addon-dispatch-proxy`.
- README: the repository URL becomes `https://github.com/torrent-tv/ha-addon#release`
  (also in the my.home-assistant.io badge). The owner's Home Assistant is migrated
  from `b34a1737_torrent_tv_proxy` to `6630818b_torrent_tv_proxy`.

## Non-goals

- No second repository. No change to the add-on image itself (`Dockerfile` build
  steps, `run.sh`). Removing `addon-install-proxy`, `add-changelog-entry.mjs`,
  `version-at-least.mjs` is step 4.

## Impact

- Users of `https://github.com/torrent-tv/ha-addon` (no `#release`) stay on the
  last release made from `main` (0.100.64 or the last before the switch) and are
  offered no further update; the README says how to move.
- The owner's proxy is interrupted once during the migration.
