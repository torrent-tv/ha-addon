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
      Not shown in this repository: no throwaway ha-addon pull request failed at either step
      (the only one, ha-addon#16, changed `config.yaml` alone and failed "config.yaml states the
      package.json version"). The same two steps failed on bad input in infra on 2026-11-10: infra#22 at "Pull request adds a changeset"
      and infra#23 at "Dependencies are pinned exactly" for `^7.8.6` (meta#336 comment
      6782567095, AC 7 and 17; those runs were deleted afterwards). Archived unticked.
- [x] 2.6 After the merge, the release of this change: a `force_push` on `refs/heads/main` by
      `torrent-tv-release[bot]` follows the `pr_merge`
      (`gh api "repos/torrent-tv/ha-addon/activity?ref=refs/heads/main"`); the changeset commit
      has its author, author time and header unchanged, sets `torrent_tv_proxy/package.json`
      and `config.yaml` to `0.100.89`, adds `## 0.100.89` to `CHANGELOG.md` and deletes the
      changeset; no commit `chore(release)` is added; the next run's `version` job logs
      `no pending changesets`; tag `v0.100.89` and its GitHub Release (body equal to the
      changelog section) exist. Check: `git show --stat <sha>`, `gh release view v0.100.89`.
      Shown: ha-addon#11 `pr_merge` 2026-11-10 16:03:14 UTC, `force_push` by
      `torrent-tv-release[bot]` at 16:03:31; d5b0c08 keeps its author and header, sets
      `config.yaml` and `package.json` to 0.100.89, adds `## 0.100.89` to `CHANGELOG.md` and
      holds no changeset; run 42850746468 `version` logs `no pending changesets`; tag
      `v0.100.89` is on d5b0c08 and its GitHub Release exists (meta#336 comment 6781047118, AC
      15; tag and Release read again 2026-11-11).
- [x] 2.7 Window: the time from `pr_merge` to `force_push`, with the `version` job's step times,
      is recorded on torrent-tv/meta#336.
      Shown: `pr_merge` 16:03:14, `force_push` 16:03:31 (17 s); `version` job from 16:03:22: app
      token 1 s, checkout 1 s, setup-node 1 s, install of the release tools 3 s, version 3 s
      (16:03:29-16:03:32); recorded on torrent-tv/meta#336 as "Version window (AC 15)" (comment
      6781047118).
- [ ] 2.8 A throwaway pull request made on a pre-rewrite tip and holding a copy of a commit
      fails "Pull request carries no copy of a commit on main" until `rebase-prs` rebases it.
      Not shown in this repository: no throwaway ha-addon pull request was made on a pre-rewrite
      tip. The check was shown in infra on 2026-11-10: infra#20, cut on the pre-rewrite tip
      66e555b, failed "Pull request carries no copy of a commit on main" until `rebase-prs`
      rebased it (run 42925924818, `rebased #20 2a74679 -> d2cb3f5`; meta#336 comment
      6782567095, AC 13; the runs were deleted afterwards). Archived unticked.
- [ ] 2.9 A pull request with two commits that each add a changeset yields two versions, two
      tags and two GitHub Releases, and its run had one `Checks` and one `Image builds` leg per
      changeset commit.
      Not shown in this repository: no merged ha-addon pull request had two commits that each
      added a changeset (ha-addon#14 had four commits and one changeset). Two such commits were
      released as two versions, two tags and two Releases in proxy#66 (v2.107.83 and v2.107.84;
      meta#336 comment 6781047118, AC 14). Archived unticked.
- [x] 2.10 Proxy reaches Home Assistant. After a proxy release, this repository's `Dependencies`
      run (event `workflow_dispatch`, input `<v>`) logs the `waiting for npm ...` lines and
      `npm serves @torrent-tv/proxy@<v> after <s> s` (or the error at 20 minutes), and pushes
      `fix(proxy)` or `feat(proxy): install proxy <v> #ttv-387` as `torrent-tv-release[bot]`. That
      commit sets `dependencies.@torrent-tv/proxy` to exactly `<v>`, changes the lock and adds a
      changeset with the proxy release notes; `version` amends it with `<a>` (`package.json`,
      `CHANGELOG.md`, `config.yaml`), and tag `v<a>` and its GitHub Release follow. Field check on
      the Home Assistant host (skill `ha-field-check`): `ha store reload`; `version_latest` is
      `<a>`; `ha apps update b34a1737_torrent_tv_proxy`; the proxy reports `<v>`.
      Shown: after proxy 2.107.81, ha-addon run 42898937642 (`workflow_dispatch`) logged the
      `waiting for npm ...` lines and `npm serves @torrent-tv/proxy@2.107.81 after 123 s` and
      pushed `fix(proxy): install proxy 2.107.81 #ttv-387` as the release App; 55858e2 sets
      `dependencies.@torrent-tv/proxy` 2.107.80 -> 2.107.81 with the lock and carries the proxy
      note in `## 0.100.91`; tag `v0.100.91` is on it. Image builds (run 42899540069) passed.
      Field check on the Home Assistant host: `version_latest` 0.100.91, update `rc=0`,
      container `aarch64-addon-torrent_tv_proxy:0.100.91 Up`, proxy log `Starting
      @torrent-tv/proxy v2.107.81` at 21:30:28, a viewer played the film from 21:30:42 with
      `dropped=0` at 21:34:07 (meta#336 comment 6781047118, AC 3). The `ha store reload` output
      and a `/healthz` answer were not kept. The `PROXY_VERSION` line of AC 3 was removed by
      commit 3dfbb91 (`AGENTS.md`).
- [x] 2.11 The first `Dependencies` run after the merge ends in one of two ways: it logs
      `no updates` and `0 open Dependabot alerts` and pushes nothing, or it pushes its commits as
      the release App with the headers of design.md, each versioned and released. `update` has
      only `contents: read` and `vulnerability-alerts: read`, no environment and no `secrets.`;
      `push` has no `npm`, `npx` or test command. The first run can end red for the `ip` alert
      (design.md, "What changes for the image") until it is dismissed.
      Shown: the first `Dependencies` run, 42898937642 (2026-11-10 20:49 UTC), ended red on the
      `ip` alert GHSA-2p57-rm9w-gvfp (`no patched version exists`), the case this item allows;
      after the alert was dismissed the dispatched runs 42907558589 and 42911275294 were green
      and pushed `install proxy 2.107.82` and `2.107.85`, each versioned and released
      (v0.100.92, v0.100.93). `update` has only `contents: read` and `vulnerability-alerts:
      read`, no environment and no `secrets.`; `push` has no `npm`, `npx` or test command
      (meta#336 comment 6781047118, AC 9(a) and 9(b)).
- [ ] 2.12 A `workflow_dispatch` of `dependencies.yml` on a throwaway branch whose
      `scripts/check.sh` exits 1 ends red; its `push` job is skipped and the activity API shows no
      `push`. The branch is deleted afterwards.
      Not shown in this repository: no failing `workflow_dispatch` was made on an ha-addon
      branch. The same check passed in infra on 2026-11-10: run 42925258556 on a throwaway
      branch whose `scripts/check.sh` ended in `&& false` ended red, `Push to main` was skipped
      and the activity API showed no push (meta#336 comment 6782567095, AC 9(c); the run was
      deleted afterwards). Archived unticked.
- [x] 2.14 The proxy's tree stays whole in the image (review of head 61e4a97). Check:
      `node torrent_tv_proxy/self-contained.cjs torrent_tv_proxy/package-lock.json @torrent-tv/proxy`
      exits 0, and exits 1 naming `fastify` and `node-abi` on the lock of 61e4a97
      (`git show 61e4a97:torrent_tv_proxy/package-lock.json`); `bash scripts/check.sh closure` runs it
      in Checks; the Dockerfile runs it before `npm ci`.
- [x] 2.15 Plan AC 9(d): a second `workflow_dispatch` of `dependencies.yml` right after a green
      run that pushed commits logs `proxy <v> is pinned; <v> is not newer` and `no updates`, and
      pushes nothing; each bot push got one `Main` run plus the run of its force-push, and no
      further one. Check: run URLs, `gh api "repos/torrent-tv/ha-addon/activity?ref=refs/heads/main"`.
      Shown: run 42924338206 (23:33 UTC, green) pushed `install proxy 2.107.88` (`push`
      23:37:56, `force_push` 23:38:10, fe8b608); run 42925428571 dispatched at 23:41:28 logged
      `proxy 2.107.88 is pinned; 2.107.88 is not newer`, `no updates` and `0 commits passed` and
      pushed nothing. Each bot push got two `Main` runs (the push and its force-push:
      42924956703 and 42924986170 for 2.107.88, 42923860269 and 42923903096 for 2.107.87) and no
      third.
- [x] 2.16 Plan AC 16, ha-addon half: once the lock holds the proxy tree, the first `Dependencies`
      run after the merge, if the `ip` alert (GHSA-2p57-rm9w-gvfp) is open here, ends red and its
      `result` job names that alert; after the coordinator App dismisses it with reason `not_used`
      (`gh api "repos/torrent-tv/ha-addon/dependabot/alerts?state=dismissed"` lists it with the
      comment) the next dispatched run is green. Check: the run URLs and the API output. If no
      alert appears, the first run's log shows `0 open Dependabot alerts` and the item is closed
      with that line.
      Shown: run 42898937642 ended red with `open Dependabot alerts no run can fix: #1
      GHSA-2p57-rm9w-gvfp ip <= 2.0.1: no patched version exists`; alert #1 was dismissed with
      reason `not_used` by `torrent-tv-coordinator[bot]` at 2026-11-10 20:53:15 UTC (`gh api
      "repos/torrent-tv/ha-addon/dependabot/alerts?state=dismissed"` lists it, read again
      2026-11-11); the next dispatched run, 42907558589, was green (meta#336 comments 6781047118
      and 6782567095, AC 16).
- [x] 2.13 Every commit on `main` after this change either matches a merged pull request commit
      or is by `torrent-tv-release[bot]` with a header ending in `#ttv-386` or `#ttv-387` and only
      the paths of design.md. Every `force_push` has actor `torrent-tv-release[bot]`.
      Shown: every commit after 32ba649 is a merged pull request commit or one of the release
      App's own commits: `fix(proxy): install proxy <v> #ttv-387` (9 commits, 2.107.81 to
      2.107.92) and `fix(deps): update dependencies #ttv-386` (0c1f771), each changing
      `torrent_tv_proxy/package.json`, `package-lock.json`, `config.yaml` and `CHANGELOG.md`
      only; all 16 `force_push` entries since 2026-11-10 16:03 UTC are by
      `torrent-tv-release[bot]`, the others are 7 `pr_merge` by the coordinator and 9 `push` by
      the release App (meta#336 comment 6781047118, AC 5, checked again against `origin/main`
      9ddc69b).
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

## 3. Test plan of the closing stage: the dependency job starts from the tip of main (2026-11-11)

Change of the stage: the job `update` of `.github/workflows/dependencies.yml` checks out `main`
(`ref: main`), so a run queued by `concurrency: dependencies` does not start from the commit that was
`github.sha` when it was started (AC-REF). The defect, with its measurements: ha-addon runs 43013341200
and 43014115480 (2026-11-11). The second one was queued behind the first, checked out the dispatch-time
commit, had its push refused, its rebase stopped on `torrent_tv_proxy` package files, and it ended
`failure`; proxy 2.107.95 reached the add-on only by a manual run. The same file is in proxy, server,
ha-addon and infra.

1. Scope and risk. In scope: the leg `workflow-refs` of `scripts/check.sh` and its synthetic cases in
   `workflow-selftest`, both in the block "The checks of the workflow files", which is byte-identical in the
   four repositories. Out of scope: the checkout of the job `push` (it takes `needs.update.outputs.base`,
   the commit the patches were made on, on purpose), the other legs (unchanged), and the behaviour of
   GitHub's `concurrency` (a platform fact, not our code). Risk is low: no torrent startup, no runtime
   code; a wrong leg either lets a stale checkout through (the defect) or refuses a correct file. Depth:
   one positive case and every negative form listed in part 3, each refusal proved by its own message.
2. Level and command (unit of the shell leg, with the fixtures built by `yq` in a scratch directory):
   `bash scripts/check.sh workflow-refs` (the real file) and `bash scripts/check.sh workflow-selftest`
   (the proof that the leg fails on bad input and passes on good). The runner needs `yq` at the
   `YQ_VERSION` of the script. Exit is `bash scripts/check.sh all`, the command `dependencies.yml` and
   `main.yml` run.
3. Cases, each one `refs_case pass|fail` in `workflow_selftest`. Positive: `ref: main` on the `update`
   checkout while the `push` checkout keeps its own ref. Negative: the `update` checkout with no `ref`;
   with `${{ github.sha }}`; with `${{ github.ref }}`; with another branch; with a tag; with an empty `ref`;
   a second `update` checkout with no `ref` after a first one at `main` (every checkout is read); the
   `push` checkout at `main` while the `update` one has none (the right job is read); the job `update`
   without a checkout (the error line says `has no actions/checkout step`; the matches are counted with
   `[...] | length`, because in yq 4.54.1 a string literal at the end of a pipe prints even with no match);
   a workflow without the job `update` and no `dependencies.yml` (each error line is checked). Boundary: an
   empty string and a missing key are both refused. Not covered: a checkout through a local action or a
   `git fetch` in a `run:` (the file has none; a new form is a new case with the change that adds it).
4. Rules and exit. The test commit keeps `bash scripts/check.sh all` and `workflow-selftest` green: it
   leaves the leg out of `all` and `main.yml` and has no case that runs the leg on the repository's own
   `dependencies.yml`, because that case fails until the fix. The failing demonstration is the leg called by
   name: before the fix, `bash scripts/check.sh workflow-refs` exits 1 in each repository with
   `a checkout of job update has ref "", not main`; after it, it exits 0. The synthetic cases pass already, and
   show the leg failing on bad input and passing on `ref: main`; turning the "no checkout" branch of the leg
   into `return 0` makes `workflow-selftest` fail. The implementer commit adds exactly these lines:
   1. `dependencies.yml`, the `update` checkout: `ref: main` under `with:`.
   2. `scripts/check.sh`, `all`: `workflow_tokens && workflow_refs && commit_msg_bot` in place of
      `workflow_tokens && commit_msg_bot`.
   3. `.github/workflows/main.yml`, job `checks`, after the tokens step:
      `- name: The dependency job starts from the tip of main` with
      `run: bash scripts/check.sh workflow-refs`.
   4. `scripts/check.sh`, `workflow_selftest`, first line of the cases of `refs_case`:
      `refs_case pass "this repository's file" ''`.
   The cases use no network and no file outside their scratch directory. The leg is a check, not a
   `node:test` test, so it has no `todo` marker.
