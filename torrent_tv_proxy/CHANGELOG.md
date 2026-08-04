## 0.3.18

- **New**: Pulls proxy 2.9.100 — a long wait for a piece reports how many peers held it, how many were asked and how much was in flight, which is the missing evidence for why a seek is slow. The keyframe index is read alongside the codec probe rather than after it, and every encoder run is numbered in the log.

## 0.3.17

- **New**: Pulls proxy 2.9.99 — a torrent can be started as soon as it is opened in the browser, before an episode is chosen, so the trackers, the peer connections and the codec probe's data are already under way by the time the viewer picks.

## 0.3.16

- **Fix**: Pulls proxy 2.9.98 — the upload is no longer raised while nobody is waiting for data, a session start no longer registers as a burst of seeks, and the per-run ffmpeg log line no longer carries the full list of cut times (about 7 KB per run on a two-hour film).

## 0.3.15

- **Fix**: Pulls proxy 2.9.97 — the generous upload added in the previous release did not reach the moment it was meant for. Only torrents with a registered reader were considered, and the head-and-tail read that a new torrent starts with does not register one, so the upload stayed near-silent for the whole of that wait.

## 0.3.14

- **Fix**: Pulls proxy 2.9.96 — the proxy now uploads generously for 25 s when a torrent is added and when the viewer seeks, so peers open their slots to it sooner. Measured before the change: a session with 96 peers connected within 2 s still spent 8.36 s of its 11.46 s start waiting for 16 MB. Every encode run also records the exact ffmpeg command line, needed to track down a failure that the message alone did not explain.

## 0.3.13

- **New**: Pulls proxy 2.9.95 — the read-ahead window is now sized in seconds of playback rather than a fixed 32 MB, everything in it is marked urgent instead of just the piece under the read head, and the store keeps it in memory instead of treating it as stale. A seek back into content already downloaded brings the whole window off disk at once. The rest of the file is filled in the background, but only while the viewer's own window needs nothing.
- **Fix**: Pulls proxy 2.9.95 — a request to `/stream` for a torrent whose metadata has not arrived answers a retryable 503 instead of holding the connection open in silence until the client gives up.

## 0.3.12

- **Fix**: Pulls proxy 2.9.93 — playback no longer dies after a seek that lands exactly on a keyframe, a session can recover when an encoder run fails instead of answering 500 forever, and the encoder stays held back once it has run too far ahead rather than being released by any passing request.

## 0.3.11

- **Fix**: Pulls proxy 2.9.92 — a seek no longer waits out delays meant for a problem that no longer exists. About 1.2 s came off every seek, and two seeks in quick succession now cause one encoder restart instead of two.

## 0.3.10

- **Fix**: Pulls proxy 2.9.91 — the encoder no longer races ahead of the viewer. It used to encode to the end of the film at up to 13x, downloading the whole torrent for someone who might watch two minutes and pushing out of memory the very pieces being played. It is now suspended two minutes ahead and released as the viewer catches up.

## 0.3.9

- **Fix**: Pulls proxy 2.9.90 — a file with MP3 audio plays again. The container is now chosen per playback session by the browser, which is the only side that knows what its decoder accepts; the addon's own segment-format setting stays the default for everything else.

## 0.3.8

- **Fix**: Pulls proxy 2.9.89 — the two remaining reasons a seek took a minute. What to download is now decided by the readers alone: ffmpeg's opening read used to claim the entire file and never give it back, which outranked everything the previous release added. And a seek now releases the segment requests it made pointless, instead of leaving the player blocked on one of them for the full 60 s hold.

## 0.3.7

- **Fix**: Pulls proxy 2.9.88 — seeking no longer downloads its way to the target. A seek left the pieces it needed selected by nobody, and every restart of the encoder re-selected the whole file from the beginning, so the swarm walked forward from the first missing piece: measured at 2.47 GB fetched over 93 s for a seek that needed a single 8 MiB piece.

## 0.3.6

- **Fix**: Pulls proxy 2.9.87 — fMP4 playback no longer freezes a few seconds in. A segment holds several fragments, and each was being told it starts where the whole segment starts, so the player kept rejecting and re-requesting the same two segments.

## 0.3.5

- **Fix**: Pulls proxy 2.9.86 — fMP4 playback starts again. ffmpeg was exiting before it wrote anything because a copied AC-3 track cannot be described until its first packet has been seen, and the muxer was being asked for the header up front; the previous release blamed the segment file names, which turned out not to be the cause.

## 0.3.4

- **Fix**: Pulls proxy 2.9.85 — fMP4 playback would not start at all in the previous release; ffmpeg refused the output because the segment muxer picks the container from the file extension and does not recognise `.m4s`.

## 0.3.3

- **Fix**: Pulls proxy 2.9.84 — fMP4 segments are now cut where the playlist says, closing the gap left in the previous release which covered MPEG-TS only.

## 0.3.2

- **Fix**: Pulls proxy 2.9.83 — playback no longer dies a few seconds in. The new segment muxer creates a file when it starts writing rather than when it finishes, and the route was serving those half-written segments; readiness is now judged by the next segment having been started.

## 0.3.1

- **Fix**: Pulls proxy 2.9.82 — segments are now cut where the playlist says they are. Previously ffmpeg picked its own cut points from a target duration while the playlist was built from the container index, and the two disagreed: on a real file segment #876 meant 1:26:50 to the player and about minute 58 to ffmpeg, so a seek into the middle landed at the end and the shown duration drifted. Also carries the stream route reporting why a read failed instead of closing the connection silently.

## 0.3.0

- **Fix**: Pulls proxy 2.9.80 — a seek backward could hang forever, because the pieces behind the previous position had been removed from the download set and nothing put them back. Also fixes two pieces being given the same memory slot under concurrent downloads, which made them fail their hash and download again without end — from outside, indistinguishable from a seek that never completes while the swarm runs at full speed.

## 0.2.99

- **New**: Pulls proxy 2.9.79 — the stream route now writes responses straight out of the torrent's shared memory, with no copy anywhere on the read path. Measured on already-downloaded data: 24 MB delivered in 298 ms instead of 1008 ms (675 Mbit/s against 200), and far steadier.

## 0.2.98

- **New**: Pulls proxy 2.9.78 — reads now cross the thread boundary as positions in shared memory instead of as bytes, removing a per-read copy (18.84 ms per 10 MB) from the thread that runs the torrent, on the path a viewer waits on.

## 0.2.97

- **Fix**: Pulls proxy 2.9.77 — commands naming a torrent that is still being added now wait for it instead of failing with "Unknown source" (which showed up as no peers and no progress on the loading screen while a magnet fetched its metadata). File claims are held per reader, so one reader finishing no longer frees the file under the others. HEAD on the stream route answers from headers instead of starting a read of the whole file.

## 0.2.96

- **Fix**: Pulls proxy 2.9.76 — a read that failed inside the torrent thread no longer looks like an empty file or hangs the reader, and request ids can no longer collide between reads and commands (which silently answered a command with another request's result).

## 0.2.95

- **New**: Pulls proxy 2.9.75 — the piece store now reports resident pieces, spills and how many reads were served from memory, so its behaviour is visible in the addon log. Memory budget is sized from the host rather than fixed, and taken as needed rather than up front.

## 0.2.94

- **Fix**: Pulls proxy 2.9.74 — playback works again. Since 2.9.71 every read came back empty and loading stopped at "Preparing HLS transcode"; torrents with a `wss://` tracker, including the site demo, took the whole proxy down.

## 0.2.93

- **Fix**: Pulls proxy 2.9.73 — file stats no longer come back empty, so the loading screen shows peers, speed and progress again.

## 0.2.92

- **Fix**: Pulls proxy 2.9.72 — the torrent is no longer deleted with its data while still being read, which had left the encoder with an empty input after 2.9.71.

## 0.2.91

- **Chore**: Pulls proxy 2.9.71 — the torrent client now runs on its own thread, so serving a segment no longer queues behind torrent work (measured: 3 MB in 0.05-0.12 s against 12-23 s for 10 MB before).

## 0.2.90

- **Chore**: Pulls proxy 2.9.70 — transfer and event-loop instrumentation to locate where a slow delivery loses its time.

## 0.2.89

- **Chore**: Pulls proxy 2.9.69 — the encoder now starts exactly where the viewer seeked, with nothing able to drag it elsewhere.

## 0.2.88

- **Chore**: Pulls proxy 2.9.68 — a seek is no longer dragged back to the position the viewer just left.

## 0.2.87

- **Chore**: Pulls proxy 2.9.67 — seek no longer encodes two minutes of content before reaching the requested position.

## 0.2.86

- **Chore**: Pulls proxy 2.9.66 — the keyframe index now covers MP4/MOV and AVI in addition to Matroska.

## 0.2.85

- **Chore**: Pulls proxy 2.9.65 — segment boundaries now come from the file real keyframe index, fixing audio-without-picture and seek that walked the whole file.

## 0.2.84

- **Chore**: Pulls proxy 2.9.64 — a seek no longer gets dragged back to the previous watch position.

## 0.2.83

- **Chore**: Pulls proxy 2.9.63 — the encoder now starts before the seek target, so the player finds the preceding keyframe it needs.

## 0.2.82

- **Chore**: Pulls proxy 2.9.62 — the seek target now comes explicitly from the browser instead of being guessed from segment requests.

## 0.2.81

- **Chore**: Pulls proxy 2.9.61 — the measurement build that holds unready segment requests up to 60 s and logs each hold outcome, to observe the iOS player real patience instead of guessing it.

## 0.2.80

- **Fix**: Pulls proxy 2.9.60 — a seek no longer restarts the encoder at the position it is already encoding, which used to destroy the work being waited for and made data appear and vanish during a seek.

## 0.2.79

- **Chore**: Pulls proxy 2.9.59 — logs now carry the proxy version on session start and state why a seek restart was allowed or held, so field reports are diagnosable without host access.

## 0.2.78

- **Fix**: Pulls proxy 2.9.58 — a single seek could leave playback stuck forever, because the encoder was restarted before the current run had produced even one segment. Restarts now wait for that first segment, so the player's playlist scan can no longer steer the encoder off target.

## 0.2.77

- **Fix**: Pulls proxy 2.9.57 — segment requests are no longer held open for up to 30 s. iOS AVPlayer enforces a ~3.5 s response-header deadline and thrashed (cancelling requests, probing other positions, resetting to the start) on every seek because of it; the proxy now answers with a retryable 503 after ~2 s instead.

## 0.2.76

- **Fix**: Pulls proxy 2.9.56 — reverts the 2.9.55 encoder-steering guard, which made seeking worse (the player could reset to the start of the file).

## 0.2.75

- **Fix**: Pulls proxy 2.9.55 — one scrub of the seek bar no longer leaves the encoder ping-ponging between positions with an empty player buffer (several segment requests from a single scrub were each steering the encoder in turn), and the transcode percent no longer sits at 0% for the whole run on video-copy sessions.

## 0.2.74

- **New**: The HLS output container is now selectable from the addon's own configuration — **Segment format**: `fmp4` (default) or `mpegts`. Home Assistant persists the choice across restarts and addon updates. `fmp4` has lower overhead and is what makes hardware encoders that do not repeat codec parameter sets (Raspberry Pi / HA Yellow `v4l2m2m`) usable; `mpegts` produces classic self-contained `.ts` segments, each valid on its own, and is the fallback if a player misbehaves on fMP4.

## 0.2.73

- **Fix**: Bump to pull proxy 2.9.54 — fixes seeking leaving playback permanently frozen. ffmpeg writes every fMP4 segment claiming to start at 0 and records the real offset in the per-run init segment instead, so post-seek segments read against the session-cached init lost their position entirely and the player discarded and re-fetched them indefinitely (buffer stuck at 0s while the transcode itself ran fine). Segments are now stamped with their true timeline position as they are served, which is what CMAF requires of an independently-addressable segment anyway. Also adds a selectable output container (--segment-format fmp4|mpegts, default fmp4); the MPEG-TS path has no init segment at all, so the same class of problem cannot occur there. No addon-side change.

## 0.2.72

- **Fix**: Bump to pull proxy 2.9.53 — fixes a timeline mismatch where the video re-encode branch's `processedSeconds` silently switched from absolute to relative-to-run mid-encode, which pinned the seek look-ahead window at the run's start segment for its whole lifetime (causing repeated unnecessary ffmpeg restarts during/after a seek) and made the buffering pill's transcode percent read as stuck near 0%. No addon-side change.

## 0.2.71

- **Fix**: Bump to pull proxy 2.9.51 — the keyframe probe that backs the seek fix (2.9.50) now runs in the background on the video re-encode path with a full 25s budget instead of sharing the copy path's 6s cap, since AVI-class containers need a full packet scan that 6s could never afford — this is the container the seek bug was originally field-diagnosed on. No addon-side change.
- **Chore**: Bump to pull proxy 2.9.52 — `npm audit` fixes (`@fastify/static` 9→10.1.2, plus three transitive deps). No addon-side change.

## 0.2.70

- **Fix**: Bump to pull proxy 2.9.50 — fixes seeking getting stuck in an infinite restart loop on some containers (observed: AVI with VBR MP3 audio), which produced nothing for ~90s until the whole session died. Seeks now snap to a real, known-decodable keyframe instead of trusting the container's own on-the-fly seek, and a circuit breaker stops auto-retrying a target that keeps failing instead of looping forever. No addon-side change.

## 0.2.69

- **New**: Bump to pull proxy 2.9.49 — the transcode-session progress endpoint now also reports the observed output bitrate, so the browser can compute a unified download/transcode/delivery playback-start ETA. No addon-side change.

## 0.2.68

- **Fix**: Bump to pull proxy 2.9.48 — fixes playback getting permanently "stuck" after a rapid sequence of seeks (multiple ffmpeg processes ended up alive concurrently, fighting over CPU and racing each other's file writes, because a seek-restart did not wait for the previous process to actually exit before spawning its replacement). Also fixes the "bytes still needed to resume" figure shown while buffering jumping up mid-poll instead of counting down (it now pins to a fixed target for one buffering episode instead of chasing a live, moving read position). No addon-side change.

## 0.2.67

- **Fix**: Bump to pull proxy 2.9.47 — fixes playback getting permanently stuck (endless manifest/first-segment re-fetch, buffer never advancing) even while the transcode itself encoded fine. Root cause: a read racing ffmpeg's still-in-progress write of the fMP4 init segment could return 0 bytes, and that empty result was cached forever as the session's init segment. No addon-side change.

## 0.2.66

- **New**: Bump to pull proxy 2.9.46 — the proxy reports resume-window byte progress (`resumeNeededBytes`/`resumeDownloadedBytes`) so the browser can show how much is left to download and the time to resume while buffering. No addon-side change.

## 0.2.65

- **New**: Bump to pull proxy 2.9.45 — HLS transcode output switched from MPEG-TS to fMP4/CMAF (`.m4s` + shared `init.mp4`). Puts codec headers (SPS/PPS) once in the init segment, so hardware encoders that don't repeat them (the CM4/Yellow `h264_v4l2m2m`) produce usable segments, plus lower overhead. Verified on hls.js; native iOS still to be confirmed. No addon-side change; the bump pulls the new proxy.

## 0.2.64

- **Fix**: Bump to pull proxy 2.9.44 — rolls WebTorrent back to the known-good 2.8.5 (pinned, with `uint8-util` pinned to 2.2.6). WebTorrent 3.x froze downloads after a seek (its piece picker crashed on pieces our seek prioritization deselected). No addon-side change; the bump pulls the new proxy.

## 0.2.63

- **Fix**: Bump to pull proxy 2.9.43 — fixes torrents stalling at the metadata/download stage on the new WebTorrent 3.x. The adaptive upload throttle no longer drops to 0 (in 3.x `throttleUpload(0)` blocks the whole swarm, not just seeding — it now keeps a minimal 8 KB/s keep-alive), and the disk-cap/stats code no longer throws on WebTorrent's null deselected pieces (null-safe downloaded helpers). No addon-side change; the bump pulls the new proxy.

## 0.2.62

- **Fix**: Bump to pull proxy 2.9.42 — fixes the crash where no torrent would load (WebTorrent 2.8.5 was incompatible with the newer `uint8-util` the global install pulled; updated WebTorrent to 3.x, the maintainer's own fix). Also removes the misdiagnosed v1-only pre-validation so v1/v2/hybrid all work. No addon-side change; the bump pulls the new proxy.

## 0.2.61

- **New**: Bump to pull proxy 2.9.41 — longer idle retention (HLS session TTL 2→10 min, torrent-data TTL 5→15 min) so a brief absence (pause, backgrounded tab, phone off for a few minutes) resumes without a cold ffmpeg restart or re-download. No addon-side change; the bump pulls the new proxy.

## 0.2.60

- **New**: Bump to pull proxy 2.9.40 — download-performance work: seek-aware piece prioritization now demotes the pieces behind the playhead so a far seek downloads the seek target first (not the undownloaded gap), and an adaptive upload throttle that defaults seeding to off, keeps a token floor while watching, and boosts only when the download is choke-starved. No addon-side change; the bump pulls the new proxy.

## 0.2.59

- **Chore**: Bump to pull proxy 2.9.39 — logs the stack of WebTorrent peer-connection warnings to diagnose a mid-file download stall (µTP null-peer NPE). No addon-side change.

## 0.2.58

- **New**: Bump to pull proxy 2.9.38 — adaptive bitrate for thin viewer links: per-rung `-maxrate`/`-bufsize` caps on software encodes, plus a viewer link-report route feeding a second budget-downshift trigger (cellular viewers get a stream their link can actually sustain). No addon-side change; the bump pulls the new proxy.

## 0.2.57

- **Fix**: Bump to pull proxy 2.9.37 — scrubbing (server-side seek) no longer hangs the player. Scattered post-seek segment requests from native players (notably iOS HLS) are debounced into a single ffmpeg restart at the settled position instead of ping-ponging the encoder. No addon-side change; the bump pulls the new proxy.

## 0.2.56

- **Fix**: Bump to pull proxy 2.9.36 — large torrents (many seasons/files) no longer fail with "message larger than max-message-size" on picking a file. The proxy now accepts request bodies as chunked binary frames (the base64 `.torrent` for a big pack is hundreds of KB) and reassembles them, with a matching 16 MB message-size advertisement covering already-open tabs. No addon-side change; the bump pulls the new proxy.

## 0.2.55

- **New**: Bump to pull proxy 2.9.34 — cold-start reduction. Transcode sessions start faster: the second redundant ffmpeg probe is gone (the session reuses the planner's probe) and the file-body start is pre-warmed so the first segment doesn't wait on torrent pieces. Startup is now logged per stage (`cold-start …: media-info=… keyframes=… create-total=…`, `first-segment ready +…ms`) — visible in the addon log. No addon-side change; the bump pulls the new proxy.

## 0.2.54

- **New**: Bump to pull proxy 2.9.33 — HDR / 10-bit tone mapping. HDR sources re-encoded to SDR are now tone-mapped (zscale+tonemap) instead of looking washed-out, when this ffmpeg build has the filters (checked at startup, logged). Whether HA's ffmpeg includes zscale/tonemap shows in the addon log line `hwaccel: HDR tone mapping available/unavailable`. No addon-side change; the bump pulls the new proxy.

## 0.2.53

- **New**: Bump to pull proxy 2.9.32 — manual quality support. The proxy now reports the source resolution in the playback plan and honours a `manualQuality` flag (encode a viewer-forced resolution exactly, budget off). Enables the new player quality menu. No addon-side change; the bump pulls the new proxy.

## 0.2.52

- **New**: Bump to pull proxy 2.9.31 — the realtime transcode budget. At startup it picks the output resolution (not just the libx264 preset) from the benchmark, so a weak host (e.g. this Home Assistant box) encodes at a resolution it can sustain in realtime instead of stalling at full size below 1× speed; capable hosts keep full resolution. At runtime it steps resolution down if the encoder falls behind on a heavy passage — but only when the encoder (not the torrent download) is the limit. No addon-side change; the bump pulls the new proxy.

## 0.2.51

- **New**: Bump to pull proxy 2.9.30 — the proxy now converts external subtitle files (.srt/.ass) to WebVTT itself, decodes their encoding (UTF-8/Windows-1251), and detects the subtitle language from the text with an n-gram model (distinguishes Russian from Ukrainian and other languages), reporting it to the player. Pairs with the matching server release; must be updated before that server version so external subtitles keep loading.

## 0.2.50

- **New**: Bump to pull proxy 2.9.29 — magnet metadata polling (a magnet whose swarm metadata takes a moment no longer fails on the first paste), transcode output frame rate follows the source instead of a fixed 24 fps (25/30 fps content plays without judder), and a global disk cap with LRU eviction of idle torrents (default min(10 GB, half free disk); protects small hosts from a full disk taking down Home Assistant).

## 0.2.49

- **Fix**: Bump to pull proxy 2.9.27 — a magnet whose infoHash matches an already-loaded torrent no longer fails with "Cannot add duplicate torrent"; both source keys now share the one swarm.

## 0.2.48

- **New**: Bump to pull proxy 2.9.26 — track selection and embedded subtitles. The playback plan lists every audio and subtitle track (language, title, default flag); `POST /api/transcode-sessions` accepts `audioTrackIndex` so the viewer can switch audio languages; new `GET /api/subtitles` extracts embedded text subtitles as WebVTT; new `GET /api/sources/:key/files` lists a source's files (groundwork for magnet input). Announce log lines no longer include the tracker passkey. Pairs with server 0.8.36 (audio menu + embedded subtitles in the captions menu).

## 0.2.47

- **New**: Bump to pull proxy 2.9.25 — observability. `/healthz` and `/health` now report the proxy version (so a stale-proxy delivery like 0.2.45's can be detected remotely); the addon log now explains zero-peer torrents (per-torrent added line with the `private` flag and tracker count, tracker warnings/rejections, and each announce response with the seeder/leecher counts the tracker returned); the `MaxListenersExceededWarning [Ssdp]` log flood from the 10-port WebRTC UDP mapping is fixed.

## 0.2.46

- **Fix**: Actually deliver proxy 2.9.24. The 0.2.45 image was built before 2.9.24 reached the npm registry, so it silently kept proxy 2.9.22 — whose blocking codec-probe request breaks cold-start playback against the current server (the browser expects the fast `pending` + poll protocol from 2.9.24 and times out after 60 s with "Data channel request timed out"). This rebuild installs the published 2.9.24.
- **New**: uTP peer connections. `utp-native` ships no musl prebuilds (HA base images are Alpine), so uTP was disabled on every host and peers were reachable over TCP only. The Dockerfile now compiles it from source at build time (toolchain was already present); best-effort — if the build fails, the proxy keeps working with TCP-only peers.

## 0.2.45

- **New**: Bump to pull proxy 2.9.24 — IPv6-first, cold-start fix, and disk hygiene. (1) IPv6: a second IPv6-capable STUN server so a proxy with global IPv6 gathers a v6 candidate, allowing a direct (NAT-free) connection to v6-native viewers (e.g. on cellular); candidate logs now tag address scope. (2) Cold-start: the codec-probe request returns quickly while the file header downloads (browser polls instead of one blocking request), so a torrent whose peers are still connecting plays once the header arrives instead of failing after 60 s (requires server 0.8.24, already live). (3) Disk hygiene: idle torrents are removed with their on-disk store after a 5-min idle TTL, and orphaned data from a previous hard kill is swept at startup — so downloaded files no longer pile up on the host. No behaviour change for IPv4-only proxies.

## 0.2.44

- **New**: Bump to pull proxy 2.9.23 — symmetric-NAT port prediction for WebRTC. On a symmetric NAT the proxy now offers predicted external ports (base + delta·k) alongside its real srflx candidate, so a viewer can reach it without manual port forwarding when the NAT increments ports predictably. No effect on cone NATs. Diagnostic logging included.

## 0.2.43

- **Fix**: Bump to pull proxy 2.9.22 — fixes a proxy crash on repeat/remote WebRTC sessions (an unguarded WebRTC error took down the whole process and restarted the addon). The WebRTC UDP port is now held by one persistent ICE mux listener for the proxy's lifetime, so reconnects and concurrent viewers share the single fixed port without failing to gather ICE candidates.

## 0.2.42

- **Chore**: Bump to pull proxy 2.9.21 — adds a per-poll `[stats]` log (peers, download speed, file %, header bytes) so a cold-start torrent download stall is visible in the addon log. Diagnostic only.

## 0.2.41

- **Chore**: Bump to pull proxy 2.9.20 — the proxy classifies its home NAT at startup (queries two public STUN servers and compares the external port) and logs whether it is endpoint-independent (cone — WebRTC fixed-port mapping suffices) or symmetric (would need port prediction). Diagnostic; no behaviour change to playback.

## 0.2.40

- **Chore**: Bump to pull proxy 2.9.19 — diagnostics only: the proxy logs successful port-mapping removal on shutdown, and logs full WebRTC ICE candidates, ICE-state transitions, and the selected candidate pair on connect (for verifying remote reachability and root-causing connection failures from the addon log).

## 0.2.39

- **Chore**: Bump to pull proxy 2.9.18 — WebRTC is now pinned to a single, UPnP-mapped UDP port (multiplexed across sessions via ICE UDP mux), so the proxy's WebRTC path is reachable from the internet without manual port forwarding. Declares `9090/udp` in the addon manifest (cosmetic under `host_network: true`, but documents intent).

## 0.2.38

- **Chore**: Bump to pull proxy 2.9.17 — the proxy now reports its UPnP-mapped external endpoint to the server so the server can dial back and verify the proxy is reachable from the internet (paired with server 0.8.22).

## 0.2.37

- **Chore**: Bump to pull proxy 2.9.16 — automatic UPnP/NAT-PMP port mapping at startup (opens TCP 9090 on the home router with an auto-renewed lease, removed on shutdown) so the proxy can be reached without manual port forwarding. Best-effort: a router without UPnP is logged and ignored. Disable with `--no-port-mapping`.

## 0.2.36

- **Chore**: Bump to pull proxy 2.9.15 — torrent data is now removed on graceful shutdown (`destroyStore` on each torrent + client teardown, wired into the proxy's `onClose` hook), so downloaded files no longer accumulate under the temp dir after the addon stops/restarts.

## 0.2.35

- **Chore**: Bump to pull proxy 2.9.14 — stats report header/index download progress so the browser can show the download phase's ETA and % toward the transcode phase.

## 0.2.34

- **Chore**: Bump to pull proxy 2.9.13 — video-copy start fix (0-based timeline, no start hole / audio-video desync) and keyframe-aligned segment boundaries for gap-free seeking on copied video.

## 0.2.33

- **Chore**: Bump to pull proxy 2.9.12 — fixes PTS-gap glitches (video stutter with audio still playing) on start/seek for both re-encoded video (fixed GOP) and copied video (`-copyts`); session log tags the active branch.

## 0.2.32

- **Chore**: Bump to pull proxy 2.9.11 — seek-aware torrent piece prioritization (marks the read position critical), so seeking into an undownloaded region no longer stalls ~15-18 s waiting for data.

## 0.2.31

- **Chore**: Bump to pull proxy 2.9.10 — larger adaptive-preset speed margin (1.3 → 1.8) so the chosen preset keeps playback above realtime under real load (WebTorrent + delivery contention) instead of stalling.

## 0.2.30

- **Chore**: Bump to pull proxy 2.9.8 — faster software transcode (all cores, no upscaling) with an adaptive preset chosen by a startup benchmark (highest quality that still keeps ≥1× realtime), plus a restart-cooldown that stops ffmpeg thrashing between seek positions.

## 0.2.29

- **Chore**: Bump to pull proxy 2.9.7 — playback-plan codec probe retries instead of caching an empty result, fixing black-video on files whose header downloads late (e.g. a later episode in a multi-file torrent).

## 0.2.28

- **Chore**: Bump to pull proxy 2.9.6 — duration probe returns on the container header instead of decoding the whole stream, cutting transcode-session creation from ~9.7 s to ~1 s.

## 0.2.27

- **Chore**: Bump to pull proxy 2.9.5 — segment files read with a 4 MB `highWaterMark` to deliver the body in fewer, larger chunks (cuts first-segment transfer time on busy ARM hosts).

## 0.2.26

- **Chore**: Bump to pull proxy 2.9.4 — temporary `[net-debug]` transfer-timing instrumentation (`fetchMs`/`ttfbMs`/`sendMs`/`chunks`) for diagnosing early-segment latency.

## 0.2.25

- **Chore**: Bump to pull proxy 2.9.3 — WebRTC data-channel binary response frames + send-loop backpressure. Requires server ≥ 0.8.0 (deploy the server before publishing the proxy and bumping this addon).

## 0.2.8

- **Fix**: ICE candidate filtering in proxy 2.5.6 — PNA permission dialog no longer appears when connecting from HTTPS pages. Requires proxy 2.5.6+.
- **Chore**: Bumped addon version to force Docker layer cache invalidation and install the updated proxy package.

## 0.2.7

- **Fix**: Tunnel keepalive — proxy 2.5.5 prevents Cloudflare from dropping the WebSocket tunnel after ~100 s of inactivity. Bump to pull in the fix.

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
