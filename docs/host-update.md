# Updating the add-on on a Home Assistant host

A release of this repository does not reach a host by itself: Home Assistant
reads the add-on from this repository, and each host updates its installed
add-on. This page describes how that is done and checked from an SSH session
(`ssh ha`).

## Why the commands run inside `hassio_cli`

The standalone `/usr/bin/ha` CLI on the host is not authenticated for the SSH
user: `ha store reload` and `ha apps update …` fail with
`unauthorized: missing or invalid API token` (there is no
`~/.homeassistant.yaml` and no token variable). The Supervisor runs a
`hassio_cli` container that carries its token; run the same commands inside it.
Do not create or copy a token.

The SSH user (`silentimp`) has passwordless `sudo` but is not in the `docker`
group, so every `docker` command needs `sudo`.

From the Home Assistant UI or its local terminal the plain `ha` commands work,
because a Supervisor token is present there.

## Update

```bash
ssh ha 'sudo docker exec hassio_cli ha store reload'
ssh ha 'sudo docker exec hassio_cli ha apps update b34a1737_torrent_tv_proxy'
```

Reload the store first (`ha store reload`, not `ha addons reload`), or the
update is not seen. In the UI, the Update button does the same; reload the page
if it looks stale.

If the update answers `Error: Another job is running for job group
app_b34a1737_torrent_tv_proxy`, an update is already running. It is a
multi-stage Docker build (apk and npm install, native module rebuild, image
export) of about 90 s, visible with
`ssh ha 'sudo docker logs app_builder_b34a1737_torrent_tv_proxy --tail 40'`.
Wait until the builder container is gone instead of starting the update again.

## Check

```bash
ssh ha 'sudo docker ps --filter name=app_b34a1737_torrent_tv_proxy'
ssh ha 'sudo docker exec app_b34a1737_torrent_tv_proxy node -p "require(\"/usr/local/lib/node_modules/@torrent-tv/proxy/package.json\").version"'
```

The first shows the running image, tagged with the add-on version; the second
shows the proxy version it installed, which is `dependencies` of
`torrent_tv_proxy/package.json` at that add-on version. They are two different
numbers.

Which encoder the proxy chose is in the add-on log:
`hwaccel: using hardware encoder …`, or
`… failed the HLS keyframe-alignment test; skipping` followed by
`… using software libx264`.
