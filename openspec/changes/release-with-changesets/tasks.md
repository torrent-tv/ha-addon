# Tasks: Release the add-on with changesets

Parent: torrent-tv/meta#336, stage 4 (ha-addon). Branch `ci/ttv-336-release-with-changesets`.
No `node:test` applies: no code under test changes, only workflow YAML, `package.json`, the
Dockerfile, shell steps and `scripts/check.sh`. The checks are the Checks run of this pull
request, scratch runs (design.md, Facts 3-7), throwaway pull requests and the first runs on
`main`. `[x]` is done and shown; `[ ]` waits for the pull request's CI, for the merge, or for a
blocker in design.md.

## 1. Files

- [x] 1.1 `torrent_tv_proxy/package.json`, `package-lock.json`, `.npmrc`,
      `self-contained.cjs`, `.changeset/config.json`, `.changeset/README.md`; `.node-version`; `.gitignore`.
- [x] 1.2 The Dockerfile installs the proxy with `npm ci --omit=dev --ignore-scripts` and
      compares it with `package.json`; `ARG PROXY_VERSION` is gone.
- [x] 1.3 `scripts/check.sh` with `tools`, `pins`, `config`, `closure`, `image`, `all`; `main.yml` calls
      one function per step.
- [x] 1.4 `main.yml`: `version`, `plan`, matrix `checks`, `lint`, matrix `build`, `publish`,
      `rebase-prs`, exact action versions.
- [x] 1.5 `dependencies.yml`: `update` (proxy, alerts, minor updates, tests), `push`, `result`.
- [x] 1.6 `AGENTS.md`, `README.md`, `docs/host-update.md` and this change describe the flow.
- [x] 1.7 `.changeset/ci-ttv-336-release-with-changesets.md`, in the last commit.

## 2. Acceptance criteria and checks

- [x] 2.1 No shared action, secret scan, `PROXY_VERSION`, `## Unreleased` entry or other
      removed release code remains (plan AC 10 with its extended terms). Check:
      `git grep -nE 'Unreleased|release-plan|release-prepare|release-finish|addon-install-proxy|infra-install-server|update-dependencies|push-to-main|check-changelog|changesetProblems|check-changesets|add-changeset|release-from-tags|write-version|release-tag|audit-exceptions|check-audit|create-pull-request|npm-package-ready|PROXY_VERSION|server-image|version-at-least|torrent-tv/workflows/actions|check-secrets|set-bot-identity|gh workflow run' -- . ':!openspec/changes/release-with-changesets' ':!torrent_tv_proxy/CHANGELOG.md' ':!torrent_tv_proxy/package-lock.json'`
      prints only three lines of `AGENTS.md`: the sentences that say there is no
      `ARG PROXY_VERSION` and nothing under `## Unreleased`, and the `gh workflow run` command
      the proxy's release uses; and
      `git grep -n 'HEAD:refs/heads/main' -- .github/workflows` finds two lines: the force-push of
      `version` and the push of `dependencies.yml`.
- [x] 2.2 Every dependency and every action is exact. Check: `bash scripts/check.sh pins`;
      `git grep -nE 'uses: [^ ]+@v[0-9]+$' -- .github` prints nothing. `pins` fails on `^7.8.6`,
      `config` on a different `config.yaml` version and `tools` on a changed `.node-version` (run
      locally).
- [x] 2.3 The proxy layout: the lock and `npm ci --omit=dev --ignore-scripts` give
      `node_modules/@torrent-tv/proxy/` with `package.json`, `bin/cli.js`,
      `scripts/install-native.sh` and `node_modules/node-datachannel` (design.md Fact 3).
- [x] 2.4 The Checks run of the pull request passes (run 42847383292): `Workflows are valid` (actionlint
      1.7.12), `Add-on configuration` (`frenck/action-addon-linter@v2.21.2`) and
      `Image builds`, which runs `install-native.sh` and `torrent-tv-proxy --help` and compares
      the installed proxy with `package.json` (design.md Fact 3). Check: the run's log shows
      `install-native: node v...`, `utp-native: our build at ...` and
      `proxy <v> installed, package.json names <v>`.
- [ ] 2.5 Changeset checks: two throwaway pull requests, each closed afterwards. One without a
      changeset fails "Pull request adds a changeset". One that writes `^7.8.6` into
      `devDependencies` fails "Dependencies are pinned exactly". Check: run URLs.
- [ ] 2.6 After the merge, the release of this change: a `force_push` on `refs/heads/main` by
      `torrent-tv-release[bot]` follows the `pr_merge`
      (`gh api "repos/torrent-tv/ha-addon/activity?ref=refs/heads/main"`); the changeset commit
      has its author, author time and header unchanged, sets `torrent_tv_proxy/package.json`
      and `config.yaml` to `0.100.89`, adds `## 0.100.89` to `CHANGELOG.md` and deletes the
      changeset; no commit `chore(release)` is added; the next run's `version` job logs
      `no pending changesets`; tag `v0.100.89` and its GitHub Release (body equal to the
      changelog section) exist. Check: `git show --stat <sha>`, `gh release view v0.100.89`.
- [ ] 2.7 Window: the time from `pr_merge` to `force_push`, with the `version` job's step times,
      is recorded on torrent-tv/meta#336.
- [ ] 2.8 A throwaway pull request made on a pre-rewrite tip and holding a copy of a commit
      fails "Pull request carries no copy of a commit on main" until `rebase-prs` rebases it.
- [ ] 2.9 A pull request with two commits that each add a changeset yields two versions, two
      tags and two GitHub Releases, and its run had one `Checks` and one `Image builds` leg per
      changeset commit.
- [ ] 2.10 Proxy reaches Home Assistant. After a proxy release, this repository's `Dependencies`
      run (event `workflow_dispatch`, input `<v>`) logs the `waiting for npm ...` lines and
      `npm serves @torrent-tv/proxy@<v> after <s> s` (or the error at 20 minutes), and pushes
      `fix(proxy)` or `feat(proxy): install proxy <v> #ttv-387` as `torrent-tv-release[bot]`. That
      commit sets `dependencies.@torrent-tv/proxy` to exactly `<v>`, changes the lock and adds a
      changeset with the proxy release notes; `version` amends it with `<a>` (`package.json`,
      `CHANGELOG.md`, `config.yaml`), and tag `v<a>` and its GitHub Release follow. Field check on
      the Home Assistant host (skill `ha-field-check`): `ha store reload`; `version_latest` is
      `<a>`; `ha apps update b34a1737_torrent_tv_proxy`; the proxy reports `<v>`.
- [ ] 2.11 The first `Dependencies` run after the merge ends in one of two ways: it logs
      `no updates` and `0 open Dependabot alerts` and pushes nothing, or it pushes its commits as
      the release App with the headers of design.md, each versioned and released. `update` has
      only `contents: read` and `vulnerability-alerts: read`, no environment and no `secrets.`;
      `push` has no `npm`, `npx` or test command. The first run can end red for the `ip` alert
      (design.md, "What changes for the image") until it is dismissed.
- [ ] 2.12 A `workflow_dispatch` of `dependencies.yml` on a throwaway branch whose
      `scripts/check.sh` exits 1 ends red; its `push` job is skipped and the activity API shows no
      `push`. The branch is deleted afterwards.
- [x] 2.14 The proxy's tree stays whole in the image (review of head 61e4a97). Check:
      `node torrent_tv_proxy/self-contained.cjs torrent_tv_proxy/package-lock.json @torrent-tv/proxy`
      exits 0, and exits 1 naming `fastify` and `node-abi` on the lock of 61e4a97
      (`git show 61e4a97:torrent_tv_proxy/package-lock.json`); `bash scripts/check.sh closure` runs it
      in Checks; the Dockerfile runs it before `npm ci`.
- [ ] 2.15 Plan AC 9(d): a second `workflow_dispatch` of `dependencies.yml` right after a green
      run that pushed commits logs `proxy <v> is pinned; <v> is not newer` and `no updates`, and
      pushes nothing; each bot push got one `Main` run plus the run of its force-push, and no
      further one. Check: run URLs, `gh api "repos/torrent-tv/ha-addon/activity?ref=refs/heads/main"`.
- [ ] 2.16 Plan AC 16, ha-addon half: once the lock holds the proxy tree, the first `Dependencies`
      run after the merge, if the `ip` alert (GHSA-2p57-rm9w-gvfp) is open here, ends red and its
      `result` job names that alert; after the coordinator App dismisses it with reason `not_used`
      (`gh api "repos/torrent-tv/ha-addon/dependabot/alerts?state=dismissed"` lists it with the
      comment) the next dispatched run is green. Check: the run URLs and the API output. If no
      alert appears, the first run's log shows `0 open Dependabot alerts` and the item is closed
      with that line.
- [ ] 2.13 Every commit on `main` after this change either matches a merged pull request commit
      or is by `torrent-tv-release[bot]` with a header ending in `#ttv-386` or `#ttv-387` and only
      the paths of design.md. Every `force_push` has actor `torrent-tv-release[bot]`.
- [x] 2.17 The copy check compares each commit of a pull request with the whole history of `main`,
      and a changeset must be one plain sentence. Check: `bash scripts/check.sh changesets-selftest`
      exits 0 (the earlier range comparison passes the copy, `copies` fails it; each format rule fails
      its bad input), and the step "Changeset checks fail on bad input" passes in `Checks`.
- [x] 2.18 The step "Test every commit, oldest first" of `dependencies.yml` runs `.githooks/commit-msg` on
      each prepared commit and stops at a refused one. Check: in a scratch repository with a stub `npm` and
      a stub `scripts/check.sh`, the step keeps the commit before one whose body has a line of 120
      characters and stops there, and passes a commit made by the helper `commit` (folded body); the leg
      `commit-msg-bot` of the next commit runs exactly that.
- [x] 2.19 The workflow files are checked: `workflow-pins`, `workflow-tokens`, `commit-msg-bot` and their proof on
      bad input. Check: `bash scripts/check.sh workflow-selftest` exits 0, the three legs exit 0 on `main`,
      and the steps "Workflow tools are pinned exactly", "The dependency job gives no token to package
      code, and each pushing job holds its own", "The dependency job refuses a commit message the hook
      refuses" and "Workflow checks fail on bad input" pass in `Checks`.
