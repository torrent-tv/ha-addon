# Tasks: release the add-on to branch `release`

Parent: torrent-tv/meta#336 step 3. Exact text, YAML and tests: design.md.
Merge order (design.md section 5): `.github` → ha-addon → HA migration → proxy.

## 1. torrent-tv/.github (branch `ci/ttv-336-addon-release`)

- [ ] 1.1 Failing tests first, committed alone
      (`test(release): release the add-on to a release branch #ttv-336`):
      `test/addon-release.test.mjs` P1-P9, C1-C5, B1-B4, I1-I2 and
      `test/release-actions.test.mjs` A4-A7 (design.md section 7). Record
      `node --test test/*.test.mjs`: the 24 new tests fail, every existing test passes.
- [ ] 1.2 `scripts/addon-release.mjs` with `proxy`, `commit`, `branch`, `installs`
      (design.md 2.1), using `lib.mjs` (`newestTag`, `compareVersions`,
      `readVersion`, `writeVersion`, `options`, `output`, `fail`, `git`). No change
      to existing functions of `lib.mjs`; a new helper may be appended.
- [ ] 1.3 `actions/addon-release-plan/action.yml` (2.2),
      `actions/addon-release-branch/action.yml` (2.3),
      `actions/addon-dispatch-proxy/action.yml` (2.4).
- [ ] 1.4 `actions/addon-install-proxy/action.yml:103-105`: newest tag of the
      repository (2.5); one sentence in its description that
      `addon-dispatch-proxy` replaces it.
- [ ] 1.5 `README.md` rows (2.6).
- [ ] 1.6 `node --test test/*.test.mjs` green; PR, review, merge.

## 2. torrent-tv/ha-addon (this worktree, branch `ci/ttv-336-release-branch`)

- [ ] 2.1 `.github/workflows/main.yml` as design.md 3.1.
- [ ] 2.2 `torrent_tv_proxy/config.yaml` comment, `torrent_tv_proxy/Dockerfile`
      comment, `torrent_tv_proxy/CHANGELOG.md` pointer line (3.2 items 1-3). If
      main gained a release before this merges, the frozen values are that
      release's (rebase).
- [ ] 2.3 `README.md`, `AGENTS.md`, `openspec/config.yaml` (3.2 items 5-7).
- [ ] 2.4 Push first WITHOUT `.changeset/ci-ttv-336-release-branch.md`: record
      the red "Pull request adds a changeset". Amend with the changeset (3.2 item 4),
      force-push: record green checks and "Image builds" with proxy 2.107.64.
- [ ] 2.5 One commit `ci(release): release the add-on to branch release #ttv-336`
      holding the files above and this OpenSpec change.
- [ ] 2.6 After the merge: acceptance 3 (design.md section 8).

## 3. Owner's Home Assistant

- [ ] 3.1 Migration design.md 6.1, steps 0-9; record the facts in meta#336.

## 4. torrent-tv/proxy (after proxy#48; branch `ci/ttv-336-addon-dispatch`)

- [ ] 4.1 `main.yml` add-on steps → `addon-dispatch-proxy` (design.md section 4),
      header comment, `AGENTS.md` "Release", changeset.
- [ ] 4.2 After the merge: acceptance 5 and 6, field check design.md 6.2.

## 5. Step 4 (later)

- [ ] 5.1 Remove `actions/addon-install-proxy`, `scripts/add-changelog-entry.mjs`,
      `scripts/version-at-least.mjs` and test A7 once no workflow references them.
