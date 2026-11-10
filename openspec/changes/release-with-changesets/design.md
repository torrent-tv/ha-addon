# Design: Release the add-on with changesets

Facts below were read from the files named, the GitHub documentation, or measured in scratch
git repositories and scratch npm projects on this Mac (no torrent, no network beyond the npm
registry and GitHub's API reads). Each says which. Analysis is marked. The versions, jobs and
rules shared with infra and server are those of torrent-tv/infra's design
(`openspec/changes/release-with-changesets/design.md`); this file lists what is specific here.

## Facts

1. State of `origin/main` before this change (32ba649, 2026-11-10): `config.yaml` version
   `0.100.88`; newest tag `v0.100.88` on 32ba649 (`chore(release): 0.100.88`); the top section
   of `torrent_tv_proxy/CHANGELOG.md` is `## 0.100.88` and there is no `## Unreleased` section
   and no changeset file, so nothing was converted; `ARG PROXY_VERSION=2.107.80`; no
   `package.json`; no pull request open; no ruleset (`gh api repos/torrent-tv/ha-addon/rulesets`
   returns an empty list); merge commits and squash merges are disabled. The old `main.yml` called
   `check-changelog`, `npm-package-ready`, `release-prepare` and `release-finish`; it did not
   call `addon-install-proxy`, which the proxy's own workflow calls to push
   `fix(proxy): install proxy <v>` commits here.
2. The release App `torrent-tv-release` has `actions: write`, `contents: write`,
   `pull_requests: write` and `workflows: write` (`gh api /apps/torrent-tv-release`). Secret
   `RELEASE_APP_PRIVATE_KEY` and variable `RELEASE_APP_CLIENT_ID` are on this repository's
   `production` environment. Dependabot alerts are enabled (`GET .../vulnerability-alerts`
   answered 204) and there is no open alert (`GET .../dependabot/alerts` answered an empty list).
3. Layout of the installed proxy. The first version of this change had `semver` 7.8.6 as a
   devDependency of the add-on, and the review (head 61e4a97) found that `npm` then shared it at
   `node_modules/semver` outside the proxy's directory: `fastify` (`semver ^7.6.0`) and
   `node-abi` (`semver ^7.3.5`) of the proxy's tree resolved to it, and the final stage copies
   only `node_modules/@torrent-tv/proxy`. The build still passed, because `install-native.sh`
   step 4 runs a lockless `npm install` in the proxy directory (`added 1 package, and changed 1
   package` in run 42847698605), which fetched `semver` from the registry by range. So the layout
   sentence of that version ("everything the proxy needs sits in its own node_modules") was false,
   and the image did not install the locked tree. Fix: the add-on's tool is installed under an
   alias, `"semver-tool": "npm:semver@7.8.6"`, the lock is regenerated, and
   `self-contained.cjs` reads the lock and resolves every dependency of every package under
   `node_modules/@torrent-tv/proxy` the way Node does. On the first lock it fails, naming exactly
   `fastify` and `node-abi` (run on this Mac, exit 1); on the regenerated lock it passes (308
   packages under the proxy). In a scratch project (npm 11.19.1) `npm ci --omit=dev
   --ignore-scripts` from the new lock gives `node_modules/@torrent-tv/proxy/` with
   `package.json`, `bin/cli.js` (mode 755) and `scripts/install-native.sh`, a top level without
   `semver`, and a copy of only that directory in which every dependency of its 301 packages
   resolves inside it (checked on disk; the names of Node core modules aside). The Dockerfile runs
   `self-contained.cjs` before `npm ci`, and `scripts/check.sh closure` runs it in Checks and on
   every commit of `dependencies.yml`, so a later update that moves a package out fails there. The
   `Image builds` job of this pull request runs the real build (design.md Blocker 1).
4. The proxy's `overrides` (our `utp-native` fork) do not apply when the proxy is a dependency,
   as they did not in the global install this replaces; `install-native.sh` step 4 installs
   ours into the package directory ("A global install ignores overrides").
5. `changeset status --since=origin/main` run in `torrent_tv_proxy` (scratch clone, @changesets/cli
   3.0.3) exits 1 for a commit that changes only `AGENTS.md`, and for one that changes only
   `run.sh`, and exits 0 after a changeset is added. So the directory is the root of the
   package and every pull request needs a changeset, an empty one for a change that releases nothing.
6. `changeset git-tag` run in `torrent_tv_proxy` tags `v<version>` (`v0.100.89`, `v0.101.0` in
   the scratch run), the form of the existing tags.
7. The scripts of `version`, `publish`, `rebase-prs` (main.yml) and of `dependencies.yml` ran on
   extracted copies in scratch git repositories with a bare origin: two changeset commits with a
   plain commit between give versions 0.100.89 and 0.101.0, each amended into its own commit
   with the lock, `CHANGELOG.md` and `config.yaml`; a second run finds `no pending changesets`; a
   later commit that edits the `name` line of `package.json` and the `description` line of
   `config.yaml` (the lines next to the version) is resolved and keeps the parent's version;
   `publish` skips the old release commit `32ba649`, which added a heading and has no
   `package.json`, tags and releases the two new commits in order, and finds both tags on a
   second run. The proxy-install step with `proxy-version=2.107.80` on a lock at 2.107.77 logged
   `npm serves @torrent-tv/proxy@2.107.80 after 0 s`, committed `fix(proxy): install proxy
   2.107.80 #ttv-387` and a changeset that carries the three proxy release notes between the two
   versions; version `2.107.999` (not published) logged the `waiting for npm ...` line and ended
   with the error; a version that is not newer, and a value that is not a version, were refused.
   A patch that writes `evil.txt` is refused by the `push` job; a refused push is retried
   after `git fetch` and `git rebase --onto origin/main <old base>`. The alert step ran on a
   fabricated alert list: an alert without a patched version is reported as unfixed; a raise of a
   transitive package was not exercised (no real alert exists).
8. `frenck/action-addon-linter`: the newest tag is `v2.21.2` (published 2026-11-06); the
   repository has been renamed `frenck/action-app-linter`, and GitHub redirects the old name.
9. Home Assistant follows a force-pushed branch: `supervisor/store/git.py` fetches `origin` with
   `depth: 1` and, when the tip differs, runs `git reset --hard origin/<branch>`
   (plan revision 4, F4). Not measured on the host in this stage.

## Decisions (analysis)

### Version file and its copy

`torrent_tv_proxy/package.json` holds the add-on version. `changeset version` runs with its
directory as the root, so tags stay `v<version>` and `CHANGELOG.md` stays where Home Assistant
reads it. `config.yaml`'s `version:` line is a copy: the `version` job writes it in the same
amended commit from `package.json`, and the Checks step `config.yaml states the package.json
version` (`scripts/check.sh config`) compares the two on every pull request and push. The
`package-lock.json` version lines come from `npm install --package-lock-only` in the same step.

### The proxy

`dependencies["@torrent-tv/proxy"]` is the only place that names the proxy version. The
Dockerfile compares it with the installed package and fails when they differ. The proxy
changes only through `dependencies.yml`; it is exact (`save-exact=true`, and `scripts/check.sh
pins` refuses a range).

### The version job, plan, checks, publish, rebase-prs

As in infra's design, with these differences:
- The changeset directory is `torrent_tv_proxy/.changeset`, and every `changeset` and `npm`
  command of the steps runs in `torrent_tv_proxy`.
- The amend step also writes `config.yaml`; its path check allows `torrent_tv_proxy/package.json`,
  `package-lock.json`, `CHANGELOG.md`, `config.yaml` and `.changeset/*.md`, and the conflict
  resolution takes the commit's own text of those three files and sets the parent's version back
  in each.
- The matrix of a pull request runs `checks` and `build` (the image) on each commit that adds
  a changeset and on the head, so every commit that becomes a version passes the repository's
  full pass/fail command (AGENTS.md Process 3). `lint` runs on the head.
- `publish` has no artifact: the package is private, and Home Assistant builds the image from
  `main`. A version is a commit that added a `## <version>` heading to `CHANGELOG.md` and changed
  the version in `torrent_tv_proxy/package.json`; the release commits made before the package
  existed are not versions.

### Dependencies

`dependencies.yml` is infra's and server's with these additions (its semver calls use the alias `semver-tool`):
- `workflow_dispatch` takes the optional input `proxy-version`; a scheduled run reads
  `npm view @torrent-tv/proxy version`. The step installs a version only when it is newer than
  the pinned one (`semver`), so a lost dispatch is made up by the next scheduled run.
- **Wait for npm.** Each round runs `npm view "@torrent-tv/proxy@<v>" dist.tarball` and then
  `curl -fsI <tarball>`; both must answer. The round repeats every 15 s (`NPM_POLL_SECONDS`, the
  owner's period). The limit is `NPM_WAIT_MINUTES=20`, a chosen constant, about three times the
  6.5 minutes npm took for 2.89.8 (CONTRIBUTING). No event announces that npm serves a version,
  which is why it polls; each round logs the tarball URL, the HEAD status and the seconds elapsed.
- The commit is `fix(proxy)` or `feat(proxy): install proxy <v> #ttv-387` (`feat` when the
  proxy's minor or major part rose), with a standard changeset (patch, or minor for a `feat`)
  whose text is `Install proxy <v>, which carries:` and the GitHub Release notes of every proxy
  version after the pinned one up to `<v>`, headings and rules removed. When the releases cannot
  be read the changeset says only `Install proxy <v>.` and a warning is logged.
- `npm-check-updates` runs with `--reject @torrent-tv/proxy`, so only the install step raises it.
- Every prepared commit is tested with `scripts/check.sh all` (tools, pins, config, image
  build), the repository's full pass/fail command.
- The `push` job accepts only `torrent_tv_proxy/package.json`, `package-lock.json` and
  `.changeset/*.md`; `config.yaml` is written later by `version`.

### What changes for the image

Before, each Home Assistant build resolved the proxy's dependency tree from the proxy's version
ranges (no lock reached the image). Now the lock pins the whole tree, so the add-on gets
transitive updates only from its own `dependencies.yml` run (`npm update` within the parents'
ranges), and Dependabot alerts on the proxy's tree also appear in this repository. The `ip`
advisory (GHSA-2p57-rm9w-gvfp, no patched version) will appear here once the lock exists; it is
dismissed with a reason by the coordinator after the first run shows it (owner decision,
2026-11-10). Until then that repository's `dependencies.yml` ends red.

### Copy and format checks of the changesets (2026-11-10, found by the acceptance check)

- Measured defect: `proxy` released the note of one changeset in two versions (2.107.82 and
  2.107.83). A branch cut from the old tip of `main` in the 22 s between a merge and the force-push
  of `version` kept the old commit; a plain `git rebase origin/main` kept it too, because its patch no
  longer equals the rewritten commit (the version files differ), and it added the changeset again. The
  step "Pull request carries no copy of a commit on main" compared only `origin/main..HEAD` with
  `HEAD..origin/main`, which is empty once the branch sits on top of `main`.
- A check of the path (`git log origin/main --diff-filter=A -- <changeset>`) does not see it either:
  the rewritten commit never holds the changeset, so `main` has never added that path (measured on
  `.changeset/docs-ttv-336-link-shared-rules.md`). The step compares each commit of the pull request
  with the whole history of `main` by author e-mail, author time and subject, which a rewrite and a
  rebase keep. `bash scripts/check.sh copies <base> <head>`.
- Measured defect: a changeset in the old `- **Fix**: ...` format was released as
  `- - **Fix**: ...` (proxy 2.107.85). The step "Changesets of the pull request are one plain
  sentence" (`bash scripts/check.sh changeset-format <base> <head>`) refuses a changeset whose text
  starts like a list, heading, bold label, quote or code, has more than one paragraph, has no text, or
  has text and no package. An empty changeset (`changeset add --empty`) is allowed only where
  `CHANGESET_EMPTY_ALLOWED=yes` (ha-addon). The rule is in `torrent-tv/workflows` CONTRIBUTING.md,
  "Versions and releases", item 1.
- Proof on bad input: `bash scripts/check.sh changesets-selftest` (the step "Changeset checks fail on
  bad input") builds the exact copy case in a scratch repository, shows that the earlier range
  comparison passes it and that `copies` fails it, and runs each format rule on a bad and a good file.
  The step runs before the two checks it proves, so a refusal never hides it. The self-test clears every
  variable of `git rev-parse --local-env-vars`, and a case shows that a repository named by
  `GIT_OBJECT_DIRECTORY` and `GIT_COMMON_DIR` stays unchanged.
- Known limits of the format check (reviewed 2026-11-10, not the measured defect): two sentences in one
  paragraph pass, because the number of sentences is not decidable from the text; `__name` and a wrapped
  line starting with `- ` are refused. Of the copy check: a copy whose author date was reset
  (`rebase --ignore-date`, `commit --amend --reset-author`) or whose subject was reworded is not seen.

## Blockers and unknowns

1. The `Image builds` job of the head after the review fix is the check of Fact 3; the image
   is built on a runner, not started, and not installed on a Home Assistant host.
2. Whether the bot's pushes count as repository activity for GitHub's 60-day rule on scheduled
   workflows is not documented.
3. The first run on `main` decides: the length of the `version` window (merge to force-push),
   whether `frenck/action-addon-linter@v2.21.2` takes the same `path` input as `@v2`, and whether
   the Supervisor on the owner's host follows the rewritten `main` (field check, skill
   `ha-field-check`, not part of this stage).
4. The proxy workflow must not start `dependencies.yml` here before this change is on `main`:
   `workflow_dispatch` reaches only a workflow on the default branch. The merge order is
   ha-addon before proxy.
5. `docs/host-update.md` now reads the installed proxy version from its `package.json`; the
   command was not run on a host.

## Five answers

1. The viewer: nothing changes on the page. The add-on offers a new proxy after npm serves it
   (6.5 min measured once), one `dependencies` run and one `Main` run; no pull request waits for
   a person. The image installs the tested, locked proxy tree, and the build fails when the lock does not keep it whole.
2. Cost to build: one pull request, about 1000 lines of workflow, `scripts/check.sh`, no other
   script.
3. Cost to maintain: the `version`, `rebase-prs` and `dependencies.yml` steps are copies of
   those in proxy, server and infra and change together (AGENTS.md Process 6); each alert that
   does not apply costs one dismissal; each merge costs a re-review of every rebased pull
   request.
4. Alternatives: a separate release commit, a "Version Packages" pull request and install pull
   requests were withdrawn by the owner; `npm install --global` from a version file would keep
   the unlocked tree.
5. Undo: revert this pull request. Rewritten history is not restored, but tags and versions
   stay valid, and the Dockerfile and workflows return to `ARG PROXY_VERSION`.
