## 0.2.5

- Fix Docker layer caching: addon version now automatically busts the npm install cache on every rebuild, ensuring the latest proxy package is always installed.

## 0.2.4

- Reverted Node.js upgrade complexity; addon now uses the Node.js version bundled with the HA base image.

## 0.2.0

- **New**: WebRTC P2P tunnel architecture — the proxy now opens a persistent WebSocket tunnel to the server on startup. Video streams are delivered directly from Home Assistant to the browser over a WebRTC data channel, with the server acting only as a signalling relay.
- **Removed**: `public_base_url` config option — no longer needed with the tunnel-based architecture.
- **Changed**: Default `server_url` is now `https://webauth.courses`.
- **Fixed**: Startup crash when `public_base_url` was absent from the saved addon configuration.

## 0.1.7

- Previous stable release with HTTP heartbeat-based proxy registration.
