# Design: add-on releases on branch `release`, proxy releases by dispatch

torrent-tv/meta#336 step 3. Repositories: `torrent-tv/.github` (scripts, actions,
tests), `torrent-tv/ha-addon` (this change), `torrent-tv/proxy` (dispatch).

## 1. Decisions

### 1.1 What `main` holds

| File on `main` | Value | Why |
|---|---|---|
| `config.yaml` `version` | stays `"0.100.64"` (the last release made from `main`), with a comment that the release writes the version on branch `release` | the add-on linter needs a valid version; `writeVersion` (`.github` `scripts/lib.mjs`) needs an `x.y.z` line to replace; a user of the old URL (no `#release`) keeps a working add-on |
| `Dockerfile` `ARG PROXY_VERSION` | stays `2.107.64`; it is the **least** proxy any release installs | a published version, so `docker build` of `main` works; a PR may raise it on purpose (a Dockerfile change that needs a newer proxy); and it is what the old `addon-install-proxy` raises during the switch (section 5) |
| `CHANGELOG.md` | frozen, one pointer line on top | the release never reads `main`'s changelog; it reads the previous release tag's |

The proxy a release installs is
`max(ARG on main, ARG at the newest v* tag, dispatched version)`. The newest tag
is the newest by version in the whole repository (tags live on `release`).

### 1.2 The release commit

For main commit M (the commit checked out):
- R = child of M, message `chore(release): <version>`, author/committer
  `github-actions[bot] <41898282+github-actions[bot]@users.noreply.github.com>`.
- R changes exactly `torrent_tv_proxy/config.yaml` (version), `torrent_tv_proxy/Dockerfile`
  (the one `ARG PROXY_VERSION=` line) and `torrent_tv_proxy/CHANGELOG.md`:
  `## <version>\n\n<notes>\n\n` + the CHANGELOG of the previous release tag
  (`git show <last-tag>:torrent_tv_proxy/CHANGELOG.md`). For the first release the
  previous tag is `v0.100.64` on `main`, whose changelog is the full history; the
  frozen pointer line of `main` never reaches `release`.
- Because R is a child of M, `git tag --contains M` names R's tag and
  `git merge-base <tag> main` is M (step-1 FACT 18), so `release-from-tags.mjs
  --last-tag newest` plans the next release from M, and a re-run on M reports
  "already".

### 1.3 Order inside a release run

plan (proxy, version, notes) → commit R → `docker build` R and check the proxy in
it → `release-tag` (tag R, push the tag alone, GitHub Release) →
`addon-release-branch` (force-push `release` to the newest tag's commit).

Tag before branch: if the branch push fails, the next run (any trigger) finds the
tag and only moves the branch. Branch before tag would let a re-run plan the same
version again on a different commit, and Home Assistant would not offer it (same
version).

### 1.4 Dispatch handling

- Receiver: `repository_dispatch` `types: [proxy-release]`, payload
  `{version, bump, tag, notes}` (step-1 interface). The run checks out
  `refs/heads/main` at the time it runs, not the event's SHA, so a dispatch that
  waited in the `release` queue never releases a `main` older than the newest
  release.
- Skip (no tag, no commit; branch sync still runs) when the newest tag already
  installs `>= version`.
- Least step = `max(payload bump, patch, workflow_dispatch bump)`. Extra notes
  (before the changesets): `- **Fix**: Install proxy <v>, which carries:` (`New`
  when the payload bump is `minor`), then the proxy notes indented by two spaces.
  `--tagged-head ignore`, so an unchanged `main` still releases.
- npm must serve the version (`npm-package-ready.mjs`), checked on the add-on side
  too, so a manual or replayed dispatch cannot build an add-on that fails on the
  host.
- Sender (`addon-dispatch-proxy`): waits for npm, sends the event with the release
  app token, waits until `release`'s head carries a `v*` tag and its Dockerfile
  installs `>= version`; fails at once when the add-on's `repository_dispatch` run
  started after the send ends `failure`/`cancelled`. Token:
  `POST /repos/{owner}/{repo}/dispatches` needs the GitHub App repository
  permission **Contents: write** (docs "Permissions required for GitHub Apps",
  Contents); reading the run needs **Actions: read**. The proxy already asks for
  both (`permission-contents: write`, `permission-actions: read`).

### 1.5 Guard against dropping released changes

`addon-release.mjs commit` refuses unless the main commit of `--last-tag` (the tag
commit itself when it is on `main`, else its first parent) is an ancestor of HEAD.

### 1.6 Branch `release`

- Written only by `addon-release.mjs branch`:
  `git push --force-with-lease=refs/heads/release:<sha read> origin <newest tag commit>:refs/heads/release`
  (`<sha read>` empty when the branch does not exist yet).
- Refuses when the branch's current commit is named by no `v*` tag (someone wrote
  it by hand): the error says how to point it at a release tag.
- Never moves to a lower version: the target is always the newest tag by version.
- Home Assistant's `pull` fetches with a `+` refspec and resets to
  `origin/<branch>` (meta#336 first comment, FACT 1-2), so a force-pushed branch
  updates.
- R differs from M only in three files outside `.github/workflows/`, so pushing it
  with `GITHUB_TOKEN` (no `workflows` permission) is allowed. Pushes and tag pushes
  by `GITHUB_TOKEN` start no workflow.
- No manual bootstrap: the first release after the switch creates `release`, with
  `v0.100.64` (or the newest tag at merge time) as `last-tag`.

## 2. `torrent-tv/.github`

### 2.1 `scripts/addon-release.mjs` (new; imports from `lib.mjs`)

```
node addon-release.mjs proxy --dir <addon dir> --extra-notes <out file> [--bump auto|patch|minor] [--payload <json file>]
  outputs: newest-tag, installed, proxy-version, bump, skip
node addon-release.mjs commit --dir <addon dir> --version <x.y.z> --proxy-version <x.y.z> --last-tag <vX.Y.Z> --notes <file>
  outputs: commit
node addon-release.mjs branch [--branch release] [--remote origin]
  outputs: tag, commit, pushed
node addon-release.mjs installs --version <x.y.z> [--dir torrent_tv_proxy] [--branch release] [--remote origin]
  exit 0 = done (outputs tag, installed); exit 1 = not yet (prints why)
```
Errors: `fail(message)` (`::error::`, exit 1), no outputs.

`proxyVersionOf(text, file)`: exactly one line `/^ARG PROXY_VERSION=(\d+\.\d+\.\d+)$/m`,
else throw `<file> has no single ARG PROXY_VERSION=x.y.z line`.

**proxy**
1. `newest = newestTag()`; none → fail `no v* release tag`.
2. `installed = proxyVersionOf(git show <newest>:<dir>/Dockerfile)`,
   `floor = proxyVersionOf(<dir>/Dockerfile in the workspace)`.
3. Payload (if given): JSON object; `version` `^\d+\.\d+\.\d+$`; `tag === "v"+version`;
   `bump` in `none|patch|minor`; `notes` a string. Otherwise fail naming the field
   (`payload version "2.107" is not x.y.z`, …).
4. Payload and `compareVersions(installed, payload.version) >= 0`: write an empty
   extra-notes file; output `skip=true proxy-version=<installed> bump=none`; log
   `add-on <newest> already installs proxy <installed>; proxy <v> needs no release`.
5. `target = max(floor, installed, payload?.version)`.
6. `target > installed`: extra =
   `- **<New if payload.bump==minor else Fix>**: Install proxy <target>` + (`, which carries:\n` + each notes line prefixed by two spaces, when the payload has notes; else `.`) + `\n`;
   `bump = max(input bump (auto→none), payload bump, patch)`.
   Otherwise extra is empty and `bump` is the input bump.
7. Output `newest-tag installed proxy-version bump skip=false`; `bump` is printed
   as `auto` when it is `none`.

**commit**
1. Validate the arguments; notes file non-empty; `git status --porcelain` empty.
2. Guard 1.5.
3. `writeVersion(<dir>/config.yaml, version)`, then `readVersion` must give it.
4. Replace the one `ARG PROXY_VERSION=` line; `proxyVersionOf` must give `--proxy-version`.
5. `prev = git show <last-tag>:<dir>/CHANGELOG.md` (CRLF→LF, `""` if absent); fail
   if `prev` already has `## <version>`; write
   `## <version>\n\n<notes trimmed>\n` + (`\n` + prev without leading blank lines, when prev is non-empty).
6. `git add` the three files, `git commit -q -m "chore(release): <version>"`; check
   `git diff --name-only HEAD^ HEAD` is exactly those three; output `commit=<HEAD>`.

**branch**
1. `git fetch -q --tags <remote>`; `newest = newestTag()`, `target = <newest>^{commit}`.
2. `current` = sha of `git ls-remote <remote> refs/heads/<branch>` or empty.
3. `current === target`: output `pushed=false`, log `<branch> is at <newest>`.
4. `current` named by no `v*` tag (`git tag --points-at <current> --list 'v[0-9]*'`):
   fail `refs/heads/<branch> is at <sha7>, which no v* tag names; only the release job writes it: point it at a release tag (git push --force origin <tag>^{commit}:refs/heads/<branch>) and re-run`.
5. `git push --force-with-lease=refs/heads/<branch>:<current> <remote> <target>:refs/heads/<branch>`;
   output `pushed=true tag=<newest> commit=<target>`; log `<branch>: <old tag or "new"> → <newest>`.

**installs** (run in a clone with the remote; used by `addon-dispatch-proxy`)
1. `git fetch -q --tags <remote> +refs/heads/<branch>:refs/remotes/<remote>/<branch>`; a
   missing branch → exit 1 `no branch <branch> yet`.
2. `head = <remote>/<branch>`; `tag` = highest `v*` tag pointing at head; none → exit 1.
3. `installed = proxyVersionOf(git show head:<dir>/Dockerfile)`; `< version` → exit 1
   `<tag> installs proxy <installed>`; else output `tag installed`, exit 0.

### 2.2 `actions/addon-release-plan/action.yml` (new)

```yaml
name: Plan an add-on release
description: >-
  Plans a release of the Home Assistant add-on in torrent-tv/ha-addon and builds its
  release commit, a child of the main commit checked out that writes the add-on
  version into config.yaml, the proxy into the Dockerfile's ARG PROXY_VERSION, and
  CHANGELOG.md (the previous release's changelog under a new section). The version
  is the newest v* tag of the repository plus the step the commits ask for, at
  least the proxy's step for a proxy-release dispatch. The proxy is never lowered,
  and a dispatch of a proxy the newest release already installs is skipped. Nothing
  is pushed: the caller builds the image, then runs release-tag and
  addon-release-branch. With release 'false' it only names the proxy the next
  release would install and checks that npm serves it.
inputs:
  payload:
    description: client_payload of a proxy-release repository_dispatch as JSON ({version, bump, tag, notes}); empty otherwise.
    default: ""
  bump:
    description: Least release step, auto, patch or minor.
    default: auto
  addon-dir:
    default: torrent_tv_proxy
  release:
    description: "'false' only names the proxy and checks npm (pull requests)."
    default: "true"
  timeout-minutes:
    description: How long to wait for npm to serve the proxy.
    default: "20"
outputs:
  proxy-version:
    value: ${{ steps.proxy.outputs.proxy-version }}
  skip:
    description: "'true' when the dispatched proxy is already installed by the newest release."
    value: ${{ steps.proxy.outputs.skip }}
  released:
    value: ${{ steps.plan.outputs.released || 'false' }}
  already:
    value: ${{ steps.plan.outputs.already }}
  version:
    value: ${{ steps.plan.outputs.version }}
  tag:
    value: ${{ steps.plan.outputs.tag }}
  last-tag:
    value: ${{ steps.plan.outputs.last-tag }}
  notes-file:
    value: ${{ runner.temp }}/release-notes.md
  commit:
    value: ${{ steps.commit.outputs.commit }}
runs:
  using: composite
  steps:
    - name: Fetch the release tags
      shell: bash
      run: git fetch -q --tags origin
    - name: The proxy this release installs
      id: proxy
      shell: bash
      env:
        SCRIPTS: ${{ github.action_path }}/../../scripts
        DIR: ${{ inputs.addon-dir }}
        BUMP: ${{ inputs.bump }}
        PAYLOAD: ${{ inputs.payload }}
      run: |
        args=(proxy --dir "$DIR" --bump "$BUMP" --extra-notes "$RUNNER_TEMP/addon-extra-notes.md")
        if [ -n "$PAYLOAD" ]; then
          printf '%s' "$PAYLOAD" > "$RUNNER_TEMP/proxy-release.json"
          args+=(--payload "$RUNNER_TEMP/proxy-release.json")
        fi
        node "$SCRIPTS/addon-release.mjs" "${args[@]}"
    - name: Wait until npm serves the proxy
      if: steps.proxy.outputs.skip != 'true'
      shell: bash
      env:
        SCRIPTS: ${{ github.action_path }}/../../scripts
        VERSION: ${{ steps.proxy.outputs.proxy-version }}
        MINUTES: ${{ inputs.timeout-minutes }}
      run: |
        deadline=$(( $(date +%s) + MINUTES * 60 ))
        while [ "$(date +%s)" -lt "$deadline" ]; do
          if node "$SCRIPTS/npm-package-ready.mjs" "@torrent-tv/proxy@$VERSION"; then exit 0; fi
          echo "npm does not serve the @torrent-tv/proxy@$VERSION tarball yet"
          sleep 15
        done
        echo "::error::npm did not serve the @torrent-tv/proxy@$VERSION tarball within $MINUTES minutes"
        exit 1
    - name: Plan the release
      id: plan
      if: inputs.release == 'true' && steps.proxy.outputs.skip != 'true'
      shell: bash
      env:
        SCRIPTS: ${{ github.action_path }}/../../scripts
        BUMP: ${{ steps.proxy.outputs.bump }}
        # A dispatch releases even a main commit that a tag already holds.
        TAGGED_HEAD: ${{ inputs.payload != '' && 'ignore' || 'already' }}
      run: >-
        node "$SCRIPTS/release-from-tags.mjs" --last-tag newest --tagged-head "$TAGGED_HEAD"
        --bump "$BUMP" --extra-notes "$RUNNER_TEMP/addon-extra-notes.md" --notes "$RUNNER_TEMP/release-notes.md"
    - name: Build the release commit
      id: commit
      if: steps.plan.outputs.released == 'true' && steps.plan.outputs.already != 'true'
      shell: bash
      env:
        SCRIPTS: ${{ github.action_path }}/../../scripts
        DIR: ${{ inputs.addon-dir }}
        VERSION: ${{ steps.plan.outputs.version }}
        PROXY: ${{ steps.proxy.outputs.proxy-version }}
        LAST: ${{ steps.plan.outputs.last-tag }}
      run: |
        git config user.name "github-actions[bot]"
        git config user.email "41898282+github-actions[bot]@users.noreply.github.com"
        node "$SCRIPTS/addon-release.mjs" commit --dir "$DIR" --version "$VERSION" \
          --proxy-version "$PROXY" --last-tag "$LAST" --notes "$RUNNER_TEMP/release-notes.md"
    - name: Summary
      if: inputs.release == 'true'
      shell: bash
      env:
        SKIP: ${{ steps.proxy.outputs.skip }}
        PROXY: ${{ steps.proxy.outputs.proxy-version }}
        RELEASED: ${{ steps.plan.outputs.released }}
        ALREADY: ${{ steps.plan.outputs.already }}
        VERSION: ${{ steps.plan.outputs.version }}
      run: |
        {
          if [ "$SKIP" = true ]; then echo "### No add-on release: the newest release installs proxy $PROXY"
          elif [ "$RELEASED" = true ]; then
            echo "### Add-on $VERSION with proxy $PROXY"
            if [ "$ALREADY" = true ]; then echo; echo "Already released; only what is missing is finished."; fi
            echo; cat "$RUNNER_TEMP/release-notes.md"
          else echo "### No add-on release: no conventional commit since the newest tag"; fi
        } >> "$GITHUB_STEP_SUMMARY"
```

### 2.3 `actions/addon-release-branch/action.yml` (new)

```yaml
name: Point the add-on's release branch at the newest release
description: >-
  Force-pushes branch release of the repository checked out to the commit of its
  newest v* tag; Home Assistant reads the add-on from that branch. The push has a
  lease on the commit read, and only a commit that a v* tag names is replaced, so a
  hand-written branch is never overwritten. Nothing happens when the branch is
  there already, so every release run can end with it.
inputs:
  branch:
    default: release
runs:
  using: composite
  steps:
    - name: Point the release branch at the newest release
      shell: bash
      env:
        SCRIPTS: ${{ github.action_path }}/../../scripts
        BRANCH: ${{ inputs.branch }}
      run: node "$SCRIPTS/addon-release.mjs" branch --branch "$BRANCH"
```

### 2.4 `actions/addon-dispatch-proxy/action.yml` (new)

```yaml
name: Release a proxy release in the add-on
description: >-
  After a proxy release, asks torrent-tv/ha-addon to release the add-on with it:
  waits until npm serves the version, sends the repository_dispatch event
  proxy-release with {version, bump, tag, notes}, and waits until branch release,
  which Home Assistant reads, is a v* tag whose Dockerfile installs at least that
  version. Nothing is written to the add-on's main. The add-on never lowers the
  proxy and does nothing for a version it already installs, so a re-run sends the
  event again and only verifies the result.
inputs:
  github-token:
    description: >-
      Token of the torrent-tv-release app limited to torrent-tv/ha-addon: contents
      write (POST /repos/{owner}/{repo}/dispatches) and actions read (the add-on run).
    required: true
  proxy-version:
    required: true
  bump:
    description: The proxy's release step; the add-on takes at least this step.
    required: true
  notes-file:
    description: The proxy release notes.
    required: true
  timeout-minutes:
    description: How long to wait for npm, and then for the add-on release.
    default: "40"
runs:
  using: composite
  steps:
    - name: Wait until npm serves the release
      # same step as actions/addon-install-proxy "Wait until npm serves the release"
    - name: Send the release to the add-on
      id: send
      shell: bash
      env:
        VERSION: ${{ inputs.proxy-version }}
        BUMP: ${{ inputs.bump }}
        NOTES: ${{ inputs.notes-file }}
        GH_TOKEN: ${{ inputs.github-token }}
      run: |
        echo "since=$(date -u +%Y-%m-%dT%H:%M:%SZ)" >> "$GITHUB_OUTPUT"
        event="$RUNNER_TEMP/proxy-release.json"
        jq -n --arg v "$VERSION" --arg b "$BUMP" --arg t "v$VERSION" --rawfile n "$NOTES" \
          '{event_type: "proxy-release", client_payload: {version: $v, bump: $b, tag: $t, notes: $n}}' > "$event"
        # GitHub refuses a payload of 64 KB or more.
        if [ "$(wc -c < "$event")" -ge 65000 ]; then
          echo "::error::the proxy-release event for $VERSION is $(wc -c < "$event") bytes; GitHub accepts less than 64 KB"
          exit 1
        fi
        gh api repos/torrent-tv/ha-addon/dispatches --input "$event"
        echo "sent proxy-release $VERSION ($BUMP) to torrent-tv/ha-addon"
    - name: Wait for the add-on release on branch release
      shell: bash
      env:
        SCRIPTS: ${{ github.action_path }}/../../scripts
        VERSION: ${{ inputs.proxy-version }}
        TOKEN: ${{ inputs.github-token }}
        GH_TOKEN: ${{ inputs.github-token }}
        MINUTES: ${{ inputs.timeout-minutes }}
        SINCE: ${{ steps.send.outputs.since }}
      run: |
        dir="$RUNNER_TEMP/ha-addon"
        rm -rf "$dir"
        git init -q "$dir"
        git -C "$dir" remote add origin "https://x-access-token:$TOKEN@github.com/torrent-tv/ha-addon.git"
        deadline=$(( $(date +%s) + MINUTES * 60 ))
        while [ "$(date +%s)" -lt "$deadline" ]; do
          if (cd "$dir" && node "$SCRIPTS/addon-release.mjs" installs --branch release --version "$VERSION" > "$RUNNER_TEMP/installs.txt"); then
            cat "$RUNNER_TEMP/installs.txt"
            echo "### Add-on $(sed -n 's/^tag=//p' "$RUNNER_TEMP/installs.txt") on branch release installs proxy $(sed -n 's/^installed=//p' "$RUNNER_TEMP/installs.txt")" >> "$GITHUB_STEP_SUMMARY"
            exit 0
          fi
          cat "$RUNNER_TEMP/installs.txt"
          # The add-on run the event started (or a later one) failing is final.
          conclusion=$(gh api -X GET repos/torrent-tv/ha-addon/actions/runs -f event=repository_dispatch -f created=">=$SINCE" \
            --jq '[.workflow_runs[] | select(.name == "Main")][0].conclusion // ""')
          if [ "$conclusion" = failure ] || [ "$conclusion" = cancelled ]; then
            echo "::error::the add-on's proxy-release run ended $conclusion; see torrent-tv/ha-addon Actions"
            exit 1
          fi
          sleep 20
        done
        echo "::error::branch release of the add-on installs no proxy >= $VERSION after $MINUTES minutes"
        exit 1
```

### 2.5 `actions/addon-install-proxy/action.yml` (kept until step 4; one change)

Lines 103-105 become:
```bash
          git -C "$dir" fetch -q --tags origin main
          # The newest add-on release tag of the repository: on main before
          # torrent-tv/meta#336 step 3, on branch release after it.
          tag=$(git -C "$dir" tag --list 'v[0-9]*' --sort=-v:refname | head -n 1)
```
While tags are on `main` this is the same tag `git describe … origin/main` gives.
After the add-on switch it finds the tag on `release`, so a proxy release made
with this action during the switch still ends green (section 5). Description: add
one sentence that it is replaced by `addon-dispatch-proxy`.

### 2.6 README.md rows

`actions/addon-release-plan`, `actions/addon-release-branch` (ha-addon release to
branch `release`), `actions/addon-dispatch-proxy` (replaces `addon-install-proxy`
from meta#336 step 3). The `RELEASE_APP_CLIENT_ID` row: the `proxy` → `ha-addon`
token now sends a `repository_dispatch` (contents write) and reads the run
(actions read).

## 3. `torrent-tv/ha-addon`

### 3.1 `.github/workflows/main.yml` (whole file)

```yaml
name: Main

# Pull requests and pushes to main run the checks. A release builds one commit on
# top of the main commit it releases, writing the add-on version (config.yaml), the
# proxy it installs (Dockerfile ARG PROXY_VERSION) and CHANGELOG.md; that commit is
# tagged v<version> and force-pushed as branch release, which Home Assistant reads
# (https://github.com/torrent-tv/ha-addon#release). main is never written.
# A push to main releases the add-on change with the newest release's proxy; a proxy
# release arrives as the repository_dispatch event proxy-release (torrent-tv/.github
# actions/addon-dispatch-proxy) and releases main with that proxy.

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
  repository_dispatch:
    types: [proxy-release]
  workflow_dispatch:
    inputs:
      bump:
        description: Least release step (a feat still releases minor)
        type: choice
        options: [auto, patch, minor]
        default: auto

permissions:
  contents: read

jobs:
  checks:
    name: Checks
    if: github.event_name != 'repository_dispatch'
    runs-on: ubuntu-26.04
    # A newer push to a pull request makes its older checks pointless; every push to
    # main gets a group of its own, so no check of main is ever replaced.
    concurrency:
      group: ${{ github.event_name == 'pull_request' && format('checks-{0}', github.ref) || format('checks-{0}', github.run_id) }}
      cancel-in-progress: ${{ github.event_name == 'pull_request' }}
    # One step per check, so the run shows which check ran and which one failed.
    steps:
      - uses: actions/checkout@v7
        with:
          fetch-depth: 0
      - uses: actions/setup-node@v7
        with:
          node-version: 24
      - name: Commit messages follow Conventional Commits
        uses: torrent-tv/.github/actions/check-commits@main
      - name: Branch name names a task
        uses: torrent-tv/.github/actions/check-branch@main
      - name: Shell scripts and Dockerfiles use LF
        uses: torrent-tv/.github/actions/check-line-endings@main
      - name: Workflows are valid
        uses: torrent-tv/.github/actions/check-workflows@main
      - name: Pull request adds a changeset
        uses: torrent-tv/.github/actions/check-changesets@main

  lint:
    name: Add-on configuration
    if: github.event_name != 'repository_dispatch'
    runs-on: ubuntu-26.04
    steps:
      - uses: actions/checkout@v7
      - uses: frenck/action-addon-linter@v2
        with:
          path: ./torrent_tv_proxy

  build:
    name: Image builds
    needs: checks
    # A release builds its own commit; a pull request builds main's Dockerfile with
    # the proxy the next release would install, which npm must serve.
    if: github.event_name == 'pull_request'
    runs-on: ubuntu-26.04
    steps:
      - uses: actions/checkout@v7
        with:
          fetch-depth: 0
      - uses: actions/setup-node@v7
        with:
          node-version: 24
      - id: proxy
        uses: torrent-tv/.github/actions/addon-release-plan@main
        with:
          release: "false"
      - name: Build the add-on image for amd64
        env:
          PROXY: ${{ steps.proxy.outputs.proxy-version }}
        run: docker build --build-arg BUILD_VERSION=ci --build-arg PROXY_VERSION="$PROXY" torrent_tv_proxy

  release:
    name: Release
    needs: [checks, lint, build]
    # A dispatch skips checks, lint and build, and a push skips build; a skipped job
    # is not a failed one.
    if: ${{ !cancelled() && !contains(needs.*.result, 'failure') && github.event_name != 'pull_request' && github.ref == 'refs/heads/main' }}
    runs-on: ubuntu-26.04
    environment: production
    # Every release runs, one at a time, in the order the runs reach the queue. A later
    # run sees the earlier tag and releases what came after it; a run whose commit a
    # tag already holds, or whose proxy is already installed, only finishes what is
    # missing.
    concurrency:
      group: release
      cancel-in-progress: false
      queue: max
    permissions:
      contents: write
    steps:
      - uses: actions/checkout@v7
        with:
          fetch-depth: 0
          # A push releases its own commit. A dispatch or a manual run releases main as
          # it is when its turn comes, never a main older than the newest release.
          ref: ${{ github.event_name == 'push' && github.sha || 'refs/heads/main' }}
      - uses: actions/setup-node@v7
        with:
          node-version: 24

      - id: release
        uses: torrent-tv/.github/actions/addon-release-plan@main
        with:
          payload: ${{ github.event_name == 'repository_dispatch' && toJSON(github.event.client_payload) || '' }}
          bump: ${{ inputs.bump || 'auto' }}

      # What Home Assistant will build on the host, built here first.
      - name: Build the release image and check the proxy in it
        if: steps.release.outputs.released == 'true' && steps.release.outputs.already != 'true'
        env:
          VERSION: ${{ steps.release.outputs.version }}
          PROXY: ${{ steps.release.outputs.proxy-version }}
        run: |
          docker build --build-arg BUILD_VERSION="$VERSION" -t addon-release torrent_tv_proxy
          installed=$(docker run --rm --entrypoint node addon-release \
            -p 'require("/usr/local/lib/node_modules/@torrent-tv/proxy/package.json").version')
          if [ "$installed" != "$PROXY" ]; then
            echo "::error::the image of add-on $VERSION installs proxy $installed, not $PROXY"
            exit 1
          fi
          echo "add-on $VERSION installs proxy $installed"

      - if: steps.release.outputs.released == 'true'
        uses: torrent-tv/.github/actions/release-tag@main
        with:
          tag: ${{ steps.release.outputs.tag }}
          notes-file: ${{ steps.release.outputs.notes-file }}
          github-token: ${{ github.token }}

      # Every run ends here: a run whose tag push succeeded and branch push failed is
      # finished by the next run, whatever started it.
      - uses: torrent-tv/.github/actions/addon-release-branch@main
```

### 3.2 Other files

1. `torrent_tv_proxy/config.yaml`: above `version:` add
   `# The version of the last release made from main. Releases write the version on branch release (.github/workflows/main.yml); never edit it here.`
2. `torrent_tv_proxy/Dockerfile`: the comment above `ARG BUILD_VERSION`/`ARG PROXY_VERSION`
   replaces "Keep PROXY_VERSION aligned with the version recorded in CHANGELOG.md"
   with: on `main` this is the least proxy a release installs; the release writes the
   proxy it installs (the newest release's, a dispatched newer one, or this one if
   higher) on branch `release`; raise it here only to require a newer proxy. Keep
   the cache-bust explanation. The `ARG` line itself stays.
3. `torrent_tv_proxy/CHANGELOG.md`: one line above `## 0.100.64`:
   `Releases after 0.100.64 are on branch release (https://github.com/torrent-tv/ha-addon/blob/release/torrent_tv_proxy/CHANGELOG.md) and at https://github.com/torrent-tv/ha-addon/releases; this file on main is no longer updated. A pull request describes its change in .changeset/ (see AGENTS.md).`
4. `.changeset/ci-ttv-336-release-branch.md`:
   `- **Chore**: The add-on is released to branch release of this repository: add it to Home Assistant as https://github.com/torrent-tv/ha-addon#release. The repository without #release receives no further updates.`
5. `README.md`:
   - Badge URL `…supervisor_add_addon_repository/?repository_url=https%3A%2F%2Fgithub.com%2Ftorrent-tv%2Fha-addon%23release`.
   - "From repository URL" step 3: `https://github.com/torrent-tv/ha-addon#release`.
   - New subsection "Moving from the old repository URL": an install from
     `https://github.com/torrent-tv/ha-addon` gets no further updates; add the
     `#release` repository, install Torrent TV Proxy from it, copy the
     Configuration, stop the old app (both use port 9090), start the new one,
     uninstall the old app and remove the old repository. The proxy's identity is
     in the old app's data and is lost unless copied (owner hosts: AGENTS.md).
   - "Repository structure": `.changeset/` and that branch `release` is generated.
6. `AGENTS.md`:
   - Slug line: `torrent_tv_proxy`; HA shows `6630818b_torrent_tv_proxy` (repository
     `https://github.com/torrent-tv/ha-addon#release`; it was `b34a1737_…` until the
     meta#336 migration).
   - "## Changelog and release" → "## Changesets and release": `.changeset/<branch>.md`
     rules (as proxy `AGENTS.md`), main's `config.yaml` version / `PROXY_VERSION` /
     `CHANGELOG.md` meaning (1.1), the release flow (1.3), dispatch (1.4), never
     write branch `release` by hand, `workflow_dispatch` bump is a least step, re-runs
     are safe.
   - Gotchas: every `b34a1737_torrent_tv_proxy` → `6630818b_torrent_tv_proxy`;
     replace the `npm list -g` check (the runtime stage has no npm) with
     `sudo docker exec app_6630818b_torrent_tv_proxy node -e "fetch('http://127.0.0.1:9090/healthz').then(r=>r.text()).then(console.log)"`;
     "Release order" now kept by the dispatch.
7. `openspec/config.yaml` context: drop "Every release adds a CHANGELOG.md entry
   matching config.yaml version" and "Bumping config.yaml version …"; add: releases
   are generated on branch `release`; `main` is never written by a release; a PR
   adds a changeset.
8. `repository.yaml`, `run.sh`: unchanged.

One commit: `ci(release): release the add-on to branch release #ttv-336`.

## 4. `torrent-tv/proxy`

After proxy#48 (step 2) is merged; branch `ci/ttv-336-addon-dispatch`.
`.github/workflows/main.yml`, the two add-on steps at the end of the release job:
```yaml
      # The add-on is released by its own workflow on the proxy-release event;
      # its main is not written.
      - name: Token for the add-on only
        id: addon-token
        if: steps.release.outputs.released == 'true'
        uses: actions/create-github-app-token@v3
        with:
          client-id: ${{ vars.RELEASE_APP_CLIENT_ID }}
          private-key: ${{ secrets.RELEASE_APP_PRIVATE_KEY }}
          owner: torrent-tv
          repositories: ha-addon
          permission-contents: write   # POST /repos/torrent-tv/ha-addon/dispatches
          permission-actions: read     # the add-on run's result

      - if: steps.release.outputs.released == 'true'
        uses: torrent-tv/.github/actions/addon-dispatch-proxy@main
        with:
          github-token: ${{ steps.addon-token.outputs.token }}
          proxy-version: ${{ steps.release.outputs.version }}
          bump: ${{ steps.release.outputs.bump }}
          notes-file: ${{ steps.release.outputs.notes-file }}
```
Header comment of `main.yml`: "…then starts the add-on release with a
`proxy-release` dispatch". `AGENTS.md` "Release": the add-on step. Changeset
`.changeset/ci-ttv-336-addon-dispatch.md`:
`- **Chore**: A proxy release reaches the Home Assistant add-on through a release event instead of a commit to the add-on's main.`
One commit `ci(release): start the add-on release with a dispatch #ttv-336`.

## 5. Merge order and the switch

1. `torrent-tv/.github` PR (2.1-2.6). Additive except the one-line wait change in
   `addon-install-proxy`, which gives the same tag while tags are on `main`.
2. `torrent-tv/ha-addon` PR (section 3). Its merge is the first release on
   `release`: `ci(release)` → patch, `last-tag` `v0.100.64` (or the newest tag at
   merge time), notes = its changeset, proxy = `max(main, v0.100.64)` = 2.107.64.
   It creates branch `release`.
3. Owner's Home Assistant migration (section 6) to `6630818b_torrent_tv_proxy`.
4. `torrent-tv/proxy` PR (section 4), after proxy#48. Its merge releases the proxy,
   which dispatches → add-on release on `release` → field update check (6.2).

proxy#48 can merge at any point: before 2 it uses the old flow end to end; between
2 and 4 it takes the switch path below.

A proxy release between 2 and 4 (proxy#48, another proxy PR, or the daily
dependency job at 03:17 UTC) runs the old `addon-install-proxy`: it raises
`ARG PROXY_VERSION` on ha-addon `main` and pushes `fix(proxy): install proxy X`.
The new ha-addon push run releases it: `proxy` sees `main`'s ARG above the newest
release's, installs X, writes the note `- **Fix**: Install proxy X.` (so the plan
has notes without a changeset) and takes at least patch. The old action's wait
(2.5) finds that tag on `release`, so the proxy run ends green. That one commit
on `main` is the only one, and only in the switch window.

## 6. Owner's Home Assistant (agent work)

`ssh ha` lands in the SSH app container (`hostname` `a0d7b954-ssh`); `jq` is there.
Supervisor's data (`/mnt/data/supervisor` on the host) is `/data` inside
`hassio_supervisor`, so an app's data is
`hassio_supervisor:/data/apps/data/<slug>`. `hassio_cli` has `curl` and
`$SUPERVISOR_TOKEN`, no `jq`. Below, `ha …` means
`ssh ha "sudo docker exec hassio_cli ha …"`. `OLD=b34a1737_torrent_tv_proxy`,
`NEW=6630818b_torrent_tv_proxy`. The token is never printed: options go from one
Supervisor API call to the other through a pipe on the HA side.

### 6.1 Migration (after the first release on `release`)

0. Record:
   - `git ls-remote https://github.com/torrent-tv/ha-addon refs/heads/release` = `v0.100.65^{commit}`.
   - `ssh ha "sudo docker exec hassio_cli ha apps info $OLD --raw-json | jq -c '.data|{version,state,boot,watchdog,auto_update}'"`.
   - `ssh ha "sudo docker exec hassio_supervisor cat /data/apps/data/$OLD/proxy-identity.json"` → `{"id":"00f2a588-…"}` (the id is public: the server lists it).
   - `curl -s https://webauth.courses/api/proxy-clients/health | jq -c '[.clients[]|{id,name,reachable}]'` → that id, `proxy-00f2a588`.
1. Add the repository and see the app:
   - `ha store add 'https://github.com/torrent-tv/ha-addon#release'`
   - `ssh ha "sudo docker exec hassio_cli ha store repositories --raw-json | jq -c '[.data[]|{slug,source}]'"` lists `6630818b`.
   - `ha store reload`; `ssh ha "sudo docker exec hassio_cli ha apps info $NEW --raw-json | jq -c '.data|{version_latest,version}'"` → `version_latest` = the newest release, `version` null.
2. Install (builds on the host, ~90 s+): `ha apps install $NEW`. If the CLI returns
   before the build ends, poll `ssh ha "sudo docker ps -a --filter name=app_builder_6630818b"` until it is gone; then `ha apps info` shows `version`.
3. Copy options, boot, watchdog and auto-update (token included, never shown):
   ```sh
   ssh ha 'sudo docker exec hassio_cli sh -c "curl -sSf -H \"Authorization: Bearer \$SUPERVISOR_TOKEN\" http://supervisor/addons/b34a1737_torrent_tv_proxy/info" \
     | jq -c "{options: .data.options, boot: .data.boot, watchdog: .data.watchdog, auto_update: .data.auto_update}" \
     | sudo docker exec -i hassio_cli sh -c "curl -sSf -X POST -H \"Authorization: Bearer \$SUPERVISOR_TOKEN\" -H \"Content-Type: application/json\" --data-binary @- http://supervisor/addons/6630818b_torrent_tv_proxy/options"'
   ```
   → `{"result":"ok",…}`. Check equality by hashes only:
   ```sh
   ssh ha 'for s in b34a1737_torrent_tv_proxy 6630818b_torrent_tv_proxy; do sudo docker exec hassio_cli sh -c "curl -sSf -H \"Authorization: Bearer \$SUPERVISOR_TOKEN\" http://supervisor/addons/$s/info" | jq -S -c "[.data.options,.data.boot,.data.watchdog,.data.auto_update]" | sha256sum; done'
   ```
   Two equal lines.
4. Stop the old app (both bind 9090): `ha apps stop $OLD`; `ha apps info` state `stopped`.
   (Optional, only if a log investigation needs it: copy the old log first,
   `ssh ha "sudo docker exec hassio_supervisor gzip -c /data/apps/data/$OLD/proxy.log" > <research dir>/proxy-b34a1737-final.log.gz`; uninstalling deletes it.)
5. Copy the state, after the stop so it is final:
   ```sh
   ssh ha 'sudo docker exec hassio_supervisor sh -c "cd /data/apps/data && cp -p b34a1737_torrent_tv_proxy/proxy-identity.json b34a1737_torrent_tv_proxy/local-observations.json 6630818b_torrent_tv_proxy/ && sha256sum b34a1737_torrent_tv_proxy/proxy-identity.json 6630818b_torrent_tv_proxy/proxy-identity.json b34a1737_torrent_tv_proxy/local-observations.json 6630818b_torrent_tv_proxy/local-observations.json"'
   ```
   Pairwise equal hashes. `proxy.log` and `client-*.log` are not copied.
6. Start: `ha apps start $NEW`; `ssh ha "sudo docker ps --filter name=app_6630818b_torrent_tv_proxy --format '{{.Image}} {{.Status}}'"`.
7. Verify:
   - `ssh ha "sudo docker exec app_6630818b_torrent_tv_proxy cat /data/proxy-identity.json"` = the id of step 0.
   - `ssh ha "sudo docker exec hassio_cli ha apps logs $NEW" | grep -m1 'Registered as'` → `Registered as "proxy-00f2a588" (00f2a588)`.
   - `curl -s https://webauth.courses/api/proxy-clients/health | jq -c '[.clients[]|{id,name,reachable}]'` → exactly one client, the step-0 id, `reachable: true`.
   - `ssh ha "sudo docker exec app_6630818b_torrent_tv_proxy node -e \"fetch('http://127.0.0.1:9090/healthz').then(r=>r.text()).then(console.log)\""` → `{"ok":true,"version":"<proxy of the release>"}`.
   Rollback until step 8: `ha apps stop $NEW && ha apps start $OLD`.
8. Remove the old: `ha apps uninstall $OLD`; `ha store delete b34a1737`;
   `ha store repositories` no longer lists `b34a1737`.
9. Record in meta#336 (versions, hashes, the interruption window). Update
   `/Users/silentimp/.codex/home-assistant-notes.md` and ha-addon `AGENTS.md`
   (slug `6630818b_torrent_tv_proxy`).

### 6.2 "An add-on update installs" (after section 5 step 4)

1. The proxy merge's run is green, including "Wait for the add-on release on branch
   release"; the ha-addon `repository_dispatch` run is green; `release` = new tag N.
2. `ha store reload`; `ssh ha "sudo docker exec hassio_cli ha apps info $NEW --raw-json | jq -c '.data|{version,version_latest,update_available}'"` → `update_available: true`, `version_latest` = N. (With `auto_update` true, Supervisor may install it on its own schedule first; then `version` = N already and the rest still applies.)
3. `ssh ha "sudo docker exec hassio_cli ha apps changelog $NEW" | head -5` → `## N` and `Install proxy <X>, which carries:`.
4. `ha apps update $NEW`; poll `app_builder_6630818b_torrent_tv_proxy` until gone.
5. `ha apps info` `version` = N; `/healthz` version = X; the server lists the same id.

## 7. Tests (torrent-tv/.github, written first)

New file `test/addon-release.test.mjs`, style of `test/release-from-tags.test.mjs`
with `test/support/scratch.mjs` (`repository()`, `run`, `parse`). Local helpers:

- `addon(repo)`: commit `chore: start #ttv-1` with
  `torrent_tv_proxy/config.yaml` = `name: "x"\nversion: "0.100.64"\nslug: "torrent_tv_proxy"\n`,
  `torrent_tv_proxy/Dockerfile` = `FROM scratch\nARG BUILD_VERSION\nARG PROXY_VERSION=2.107.64\nRUN echo ${PROXY_VERSION}\n`,
  `torrent_tv_proxy/CHANGELOG.md` = `## 0.100.64\n\n- **Fix**: Install proxy 2.107.64.\n`; tag `v0.100.64`.
- `released(repo, version, proxy, section)`: from HEAD M, `checkout --detach`,
  commit `chore(release): <version>` setting the three files (CHANGELOG =
  `## <version>\n\n<section>\n\n` + the previous), tag `v<version>`, `checkout main`. Returns R.
- `payloadFile(repo, object)`: writes JSON into `repo.scratch`.
- `remote(repo)`: `git init -q --bare` in a temp dir, `git remote add origin <it>`,
  `git push -q origin main --tags`.
- `ok(...args)` = `run("addon-release.mjs", ...)` with status 0 → `parse(stdout)`;
  `bad(...args)` asserts status 1 and returns the text.

| # | Name | Setup | Expected |
|---|---|---|---|
| P1 | a push releases with the newest release's proxy and no extra notes | addon; `fix: x` + changeset | `proxy --dir torrent_tv_proxy --extra-notes f`: `proxy-version=2.107.64 installed=2.107.64 newest-tag=v0.100.64 skip=false bump=auto`; `f` empty |
| P2 | a dispatched newer proxy is installed with its notes nested under one bullet | addon; payload `{version:"2.107.65",bump:"patch",tag:"v2.107.65",notes:"- **Fix**: a.\n- **Fix**: b.\n"}` | `proxy-version=2.107.65 skip=false bump=patch`; `f` = `- **Fix**: Install proxy 2.107.65, which carries:\n  - **Fix**: a.\n  - **Fix**: b.\n`; `lib.changesetProblems(f)` = `[]` |
| P3 | a minor proxy release is a New entry and at least a minor add-on step | as P2 with `bump:"minor"` | `bump=minor`; `f` starts `- **New**: Install proxy 2.107.65` |
| P4 | a dispatched proxy the newest release already installs is skipped | addon; payloads 2.107.64 and 2.107.60 | both: exit 0, `skip=true proxy-version=2.107.64 bump=none`, `f` empty |
| P5 | the newest release is read from its tag off main, and never lowered to main's value | addon; M1 = `fix: x` + change; `released(repo,"0.100.65","2.107.70",…)` | push: `newest-tag=v0.100.65 installed=2.107.70 proxy-version=2.107.70`; payload 2.107.66 → `skip=true` |
| P6 | main's PROXY_VERSION above the newest release is installed with a note (the old protocol during the switch) | addon; commit `fix(proxy): install proxy 2.107.65 #ttv-91` setting the ARG to 2.107.65 | `proxy-version=2.107.65 bump=patch`; `f` = `- **Fix**: Install proxy 2.107.65.\n` |
| P7 | a dispatch whose bump is none still releases a patch | payload 2.107.65 with `bump:"none"` | `bump=patch skip=false` |
| P8 | a malformed payload is refused by field | payloads: version `"2.107"`; tag `"v2.107.66"` for version 2.107.65; bump `"major"`; notes `5`; file text `not json` | each exits 1 naming `version`, `tag`, `bump`, `notes`, `JSON`; stdout has no `proxy-version=` |
| P9 | a manual least step passes through without a proxy change | addon; `--bump minor` | `bump=minor proxy-version=2.107.64` |
| C1 | the release commit is a child of HEAD that changes only the three files | addon; M1 = `fix: x` + change(x); notes file `- **Fix**: x.\n`; `commit --dir torrent_tv_proxy --version 0.100.65 --proxy-version 2.107.65 --last-tag v0.100.64 --notes n` | `git rev-parse HEAD^` = M1; `git diff --name-only HEAD^ HEAD` = the three paths; config.yaml = the fixture with `version: "0.100.65"`; Dockerfile = the fixture with `ARG PROXY_VERSION=2.107.65`; subject `chore(release): 0.100.65`; output `commit` = HEAD |
| C2 | the changelog is the previous release's under a new section, not main's | addon; M1 change(a); `released(…"0.100.65","2.107.65","- **Fix**: a.")`; on main M2 rewrites `torrent_tv_proxy/CHANGELOG.md` to `Frozen.\n` + change(b); commit 0.100.66 with `--last-tag v0.100.65`, notes `- **Fix**: b.\n` | CHANGELOG = `## 0.100.66\n\n- **Fix**: b.\n\n## 0.100.65\n\n- **Fix**: a.\n\n## 0.100.64\n\n- **Fix**: Install proxy 2.107.64.\n`; no `Frozen.` |
| C3 | a release commit made by commit plans the next release from its main commit | addon; M1 change(a); commit 0.100.65 → R; tag v0.100.65 on R; checkout main; M2 `fix: b` + change(b) | `release-from-tags.mjs --last-tag newest`: `last-tag=v0.100.65 base=<M1> version=0.100.66`, notes only b; at M1 (`checkout M1`, default `--tagged-head`) `already=true tag=v0.100.65` |
| C4 | a HEAD that lacks the last release's main commit is refused | addon (M0); M1 change(a); `released(…"0.100.65"…)` on M1; checkout M0; commit 0.100.66 `--last-tag v0.100.65` | exit 1, text contains `v0.100.65` and `not in HEAD`; HEAD still M0 |
| C5 | a dirty tree or a Dockerfile without one PROXY_VERSION line is refused | (a) uncommitted edit to code.txt; (b) commit removing the ARG line | both exit 1 (`clean`/`ARG PROXY_VERSION`), no new commit |
| B1 | the release branch is created at the newest tag | addon; M1; R65 = `released(…"0.100.65"…)`; remote | `branch`: `pushed=true tag=v0.100.65 commit=<R65>`; `git ls-remote origin refs/heads/release` = R65 |
| B2 | the branch moves from an older release to a newer one that is not its descendant | B1, then M2 + R66 = `released(…"0.100.66"…)`; `git push -q origin --tags` | `pushed=true`; remote release = R66 |
| B3 | nothing is pushed when the branch is at the newest tag | B1, run twice | second: `pushed=false`, exit 0 |
| B4 | a branch commit that no release tag names is not overwritten | remote; push an untagged commit U as `refs/heads/release`; R65 tagged | exit 1, text names U's short sha; remote release still U |
| I1 | installs passes when the release head is tagged and installs at least the version | B1 setup; a fresh `git init` clone dir with `origin` = the bare remote; `installs --version 2.107.65` and `--version 2.107.60` | exit 0, `tag=v0.100.65 installed=2.107.65` |
| I2 | installs waits when the branch is missing, untagged or installs less | (a) no branch; (b) branch at untagged U; (c) B1 with `--version 2.107.66` | each exit 1 with `no branch`, `no v* tag`, `installs proxy 2.107.65` |

`test/release-actions.test.mjs` (action text), add:

| # | Name | Expected |
|---|---|---|
| A4 | addon-release-plan builds the release commit and pushes nothing | contains `addon-release.mjs" "${args[@]}"`, `release-from-tags.mjs`, `--last-tag newest`, `addon-release.mjs" commit`, `npm-package-ready.mjs`; no match `/git push/` |
| A5 | addon-release-branch only runs the branch command | contains `addon-release.mjs" branch` |
| A6 | addon-dispatch-proxy sends a dispatch and never pushes | contains `repos/torrent-tv/ha-addon/dispatches`, `"proxy-release"`, `npm-package-ready.mjs`, `installs --branch release`; no `git push` |
| A7 | addon-install-proxy waits for the newest tag of the repository | contains `--sort=-v:refname`; does not contain `describe --tags --abbrev=0 --match 'v[0-9]*' origin/main` |

24 new tests, all failing before the implementation (missing script → each
`run` exits 1 with "Cannot find module" and the output assertions fail; A4-A6
ENOENT; A7 the old text). Existing tests pass unchanged.

Not unit-tested, verified by CI and the field: the ha-addon workflow YAML
(actionlint in its checks), the image build and proxy check, `release-tag` on R,
the GitHub token push of `release`, the dispatch delivery, Home Assistant reading
`#release` and updating.

## 8. Acceptance of step 3

1. `node --test test/*.test.mjs` in `torrent-tv/.github` passes, with the 24 new
   tests red on the failing-test commit.
2. ha-addon PR: "Pull request adds a changeset" red without the changeset and green
   with it (record both runs); "Image builds" logs `building … with proxy 2.107.64`.
3. After the ha-addon merge: run green; `git ls-remote origin refs/heads/release` =
   `git rev-parse v0.100.65^{commit}`; `git rev-parse v0.100.65^` = the merged main
   commit; `git diff --name-only v0.100.65^ v0.100.65` = the three files;
   `git log origin/main` has no new `chore(release)`; `gh release view v0.100.65 -R torrent-tv/ha-addon` has the changeset text; `git show release:torrent_tv_proxy/CHANGELOG.md | head -3` = `## 0.100.65`.
4. Migration 6.1 done: same id on `/api/proxy-clients/health`, one client, old app
   and repository gone.
5. After the proxy merge (Acceptance 3 of meta#336): proxy run green; ha-addon
   `repository_dispatch` run green; new tag N on `release` installs the new proxy,
   its CHANGELOG section says `Install proxy <X>, which carries:`; ha-addon `main`
   has no commit after the ha-addon merge; 6.2 passes on the owner's HA.
6. Idempotency: `gh run rerun <that repository_dispatch run> -R torrent-tv/ha-addon`
   ends green with the summary "No add-on release: the newest release installs
   proxy X", no new tag, `release` unchanged.
