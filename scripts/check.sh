#!/usr/bin/env bash
# The checks that need no secret: main.yml runs them one step each, so a run shows
# which one failed; dependencies.yml runs `all` on every commit it prepares.
#
#   bash scripts/check.sh tools | pins | config | image
#   bash scripts/check.sh all
set -euo pipefail
cd "$(dirname "$0")/.."

# The add-on directory: the Home Assistant add-on, and the root of its npm package.
ADDON=torrent_tv_proxy

tools() {
  # Security rule 6: Node (.node-version) and npm (packageManager) are exact versions,
  # and the runner provides exactly those. npm is the one bundled with that Node.
  local node_want npm_want
  node_want="v$(tr -d '[:space:]' < .node-version)"
  npm_want=$(node -p 'require("./'"$ADDON"'/package.json").packageManager.replace(/^npm@/, "")')
  if [ "$(node --version)" != "$node_want" ] || [ "$(npm --version)" != "$npm_want" ]; then
    echo "::error::node $(node --version) and npm $(npm --version) run here; the repository pins node $node_want and npm $npm_want"
    return 1
  fi
  echo "node $node_want and npm $npm_want"
}

pins() {
  # Security rule 6: no range and no `latest`, in dependencies, devDependencies,
  # optionalDependencies and overrides; an `npm:` alias names an exact version.
  ADDON_PACKAGE="$ADDON/package.json" node -e '
    const pkg = JSON.parse(require("node:fs").readFileSync(process.env.ADDON_PACKAGE, "utf8"));
    const exact = /^\d+\.\d+\.\d+(-[0-9A-Za-z.-]+)?(\+[0-9A-Za-z.-]+)?$/;
    const bad = [];
    const walk = (where, value) => {
      if (typeof value === "string") {
        const spec = value.startsWith("npm:") ? value.slice(value.lastIndexOf("@") + 1) : value;
        if (!exact.test(spec)) bad.push(where + " = " + value);
      } else if (value && typeof value === "object") {
        for (const [key, inner] of Object.entries(value)) walk(where + "." + key, inner);
      }
    };
    for (const field of ["dependencies", "devDependencies", "optionalDependencies", "overrides"]) walk(field, pkg[field]);
    if (bad.length) {
      console.error("::error::not an exact version: " + bad.join("; "));
      process.exit(1);
    }
    console.log("every dependency is pinned exactly");'
}

config() {
  # package.json holds the add-on version; config.yaml, which the Supervisor reads, is a
  # copy that the `version` job writes from it (torrent-tv/meta#336).
  local want have
  want=$(node -p 'require("./'"$ADDON"'/package.json").version')
  have=$(sed -n 's/^version: "\(.*\)"$/\1/p' "$ADDON/config.yaml")
  if [ "$have" != "$want" ]; then
    echo "::error::$ADDON/config.yaml says version \"$have\"; $ADDON/package.json says $want"
    return 1
  fi
  echo "config.yaml states version $want"
}

image() {
  # The image installs the proxy with `npm ci` from package-lock.json and checks it in its
  # last stage (`torrent-tv-proxy --help`): a build that succeeds installed the locked
  # proxy and loaded its native modules. It does not start the proxy.
  docker build --build-arg BUILD_VERSION=ci "$ADDON"
}

case "${1:-}" in
  tools) tools ;;
  pins) pins ;;
  config) config ;;
  image) image ;;
  all) tools && pins && config && image ;;
  *) echo "usage: bash scripts/check.sh tools|pins|config|image|all" >&2; exit 2 ;;
esac
