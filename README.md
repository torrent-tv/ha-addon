# Torrent TV Proxy Home Assistant Add-on

This repository contains a Home Assistant add-on that installs and runs `@torrent-tv/proxy`.

No manual file copy is required for users.

[![Open your Home Assistant and add this repository](https://my.home-assistant.io/badges/supervisor_add_addon_repository.svg)](https://my.home-assistant.io/redirect/supervisor_add_addon_repository/?repository_url=https%3A%2F%2Fgithub.com%2Ftorrent-tv%2Fha-addon)

## Install from repository URL

1. Open Home Assistant.
2. Go to **Settings -> Add-ons -> Add-on Store**.
3. Click menu (three dots) in the top-right corner.
4. Click **Repositories**.
5. Paste your Git repository URL (the URL of this `ha-addon` repository).
6. Click **Add**.
7. Find **Torrent TV Proxy** in the store and click **Install**.

You can also use a My Home Assistant direct link:

`https://my.home-assistant.io/redirect/supervisor_add_addon_repository/?repository_url=https%3A%2F%2Fgithub.com%2Ftorrent-tv%2Fha-addon`

## Configure server URL in UI

`server_url` is exposed in add-on configuration and editable in UI.

1. Open add-on **Torrent TV Proxy**.
2. Open **Configuration** tab.
3. Set `server_url` (example: `http://192.168.1.10:8080`).
4. Click **Save**.

This value is defined in add-on `options`/`schema`, so users can change it directly in Home Assistant UI.

## Start

1. Open add-on **Info** tab.
2. Click **Start**.
3. Optional: enable **Start on boot** and **Watchdog**.
4. Check **Log** tab for startup status.

## Repository structure

```text
ha-addon/
  repository.yaml
  torrent_tv_proxy/
    config.yaml
    Dockerfile
    run.sh
```
