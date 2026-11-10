// Fails when the installed copy of a package is not self-contained: a package of its tree
// that npm placed outside the package's directory, because the add-on's own package.json also
// depends on it (a direct dependency at the top is shared with every package below).
// The image copies only that directory, so such a package would be missing at run time, or
// fetched afresh by a later lockless `npm install` instead of coming from the lock.
//
//   node self-contained.cjs <package-lock.json> <package name>
//
// It reads the lock only and resolves every dependency the way Node does, from the
// directory of the package that requires it, upwards.
'use strict';
const { readFileSync } = require('node:fs');

const [file, name] = process.argv.slice(2);
if (!file || !name) {
  console.error('usage: node self-contained.cjs <package-lock.json> <package name>');
  process.exit(2);
}
const packages = JSON.parse(readFileSync(file, 'utf8')).packages ?? {};
const root = `node_modules/${name}`;
if (!packages[root]) {
  console.error(`::error::${file} has no entry ${root}`);
  process.exit(1);
}

const parentOf = (dir) => dir.replace(/(^|\/)node_modules\/(@[^/]+\/)?[^/]+$/, '');
const resolve = (from, dep) => {
  let dir = from;
  for (;;) {
    const candidate = dir ? `${dir}/node_modules/${dep}` : `node_modules/${dep}`;
    if (packages[candidate]) return candidate;
    if (!dir) return null;
    dir = parentOf(dir);
  }
};

const inside = (key) => key === root || key.startsWith(`${root}/`);
const problems = [];
let checked = 0;
for (const [key, entry] of Object.entries(packages)) {
  if (!inside(key)) continue;
  checked += 1;
  const optionalPeers = Object.entries(entry.peerDependenciesMeta ?? {})
    .filter(([, meta]) => meta.optional)
    .map(([dep]) => dep);
  const wanted = [
    ...Object.keys(entry.dependencies ?? {}).map((dep) => [dep, false]),
    ...Object.keys(entry.optionalDependencies ?? {}).map((dep) => [dep, true]),
    ...Object.keys(entry.peerDependencies ?? {}).map((dep) => [dep, optionalPeers.includes(dep)]),
  ];
  for (const [dep, optional] of wanted) {
    const found = resolve(key, dep);
    if (found === null) {
      if (!optional) problems.push(`${key} needs ${dep}, which the lock does not hold`);
    } else if (!inside(found)) {
      problems.push(`${key} needs ${dep}, which resolves to ${found}, outside ${root}`);
    }
  }
}
if (problems.length) {
  console.error(`::error::the tree of ${name} is not self-contained in ${file}:`);
  for (const problem of problems) console.error(`  ${problem}`);
  process.exit(1);
}
console.log(`the tree of ${name} is self-contained: ${checked} packages under ${root}`);
