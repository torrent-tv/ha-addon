## 0.60.0

- **New**: Pulls proxy 2.69.0. A subtitle track is no longer given a language the text cannot support. Measured over Wikipedia extracts in seventeen languages: the detector needs 650 characters of Russian before its answer stops walking between Bulgarian, Serbian and Russian, while Korean is settled by 40 — so the figure is now the candidate language's own, and an answer that does not survive losing half the text is refused as well. Instead of a wrong label the track shows Unknown, and the reading is re-taken as the film downloads: the label moves onto the right language once enough of the file has arrived.
- **Fix**: A Russian subtitle FILE was reported as English, because the detector was handed the raw `.ass` — which is half Latin markup — while the markup-free text it was about to serve sat beside it. All three places a language is read now read the words and nothing else.

## 0.59.0

- **New**: Pulls proxy 2.68.0. The background fetch of a soundtrack now stands aside after a stall, not only during one. On a swarm with no spare capacity the earlier rule still let it take bandwidth between stalls — and the stalls were the proof there was none spare. Measured on 2026-08-31: 200-600 KB/s delivered against the 399 KB/s the film eats, a seek that waited 101 seconds for a single piece, and the picture standing still 145.6 seconds before the player gave up. On a starving swarm the fetch now stops altogether.

## 0.58.0

- **New**: Pulls proxy 2.67.0. A soundtrack shipped as its own file beside the picture is now fetched WHOLE from the moment the viewer opens the episode, so switching to it does not wait for the swarm — in the field on 2026-08-31 the first piece of one took 27.7 seconds to arrive, which is longer than a switch is willing to wait, and the first switch to the Russian dub failed for exactly that reason while the second succeeded. The fetch uses only the room the viewer's own reading leaves: it proceeds only while no reader on the torrent is waiting for a piece, and that is re-asked before every piece. What plays now always comes first.
- **Fix**: The speed a quality step had to reach to survive the swarm was computed as thousands — `a step must run at 4422.00x` — and every step was refused against it. Several readers walk one file and a missing piece blocks all of them, so their waits were counted as separate interruptions arriving 0.00 s apart; they are merged into one stall now, and the interval between stalls is measured over the stretch the encoder actually runs.
- **New**: A wait for a piece now records what the WHOLE torrent received while that piece was missing, beside what the piece itself managed, and the peer count. Bytes arriving briskly throughout mean the swarm had capacity and the piece was stuck behind the wire that reserved it; bytes barely moving mean there was nothing to be had. The two could not be told apart before.

## 0.57.0

- **New**: Pulls proxy 2.66.0. A soundtrack that ships as its own file beside the picture — `Rus Sound/<name>.mka`, the common shape for a dub — is offered in the audio menu like any track inside the video, and switching to it is the player changing rendition rather than anything being rebuilt. No new container class was needed: a `.mka` is Matroska and the reader that serves the picture serves it, which also means its language, title, channel count and flags come from the file itself. The audio menu now reads those flags — commentary, audio description, original language, unusable — none of which appear in ffmpeg's banner, so a director's commentary and the film were indistinguishable in it before. A session also holds a claim on every file it reads, closing a case where the disk sweep could delete a soundtrack from under a running encoder. The warm-up that runs when a viewer picks a file now warms what lies beside it too — a text subtitle file whole, since it is smaller than one piece of the torrent, and a soundtrack's head and tail — so neither the playback plan nor the browser is the first thing to ask the swarm for them. And once the encoder is as far ahead of the viewer as it is allowed to get — the one moment the swarm's capacity is demonstrably spare — the soundtrack file is fetched whole, so a later switch to it does not wait: it is a twentieth of the picture's size, and having it on disk is the difference between an instant switch and one that pays for its own first pieces.

## 0.56.9

- **New**: Pulls proxy 2.64.9. Two readings that between them name what is leaking. The memory line now says the SHAPE of the anonymous memory, read per mapping from `/proc/self/smaps`: one growing `[heap]` means the allocator keeps what we free and on musl there is nothing to ask; many large anonymous mappings mean piece buffers, and whether their count tracks the store decides whether anything retains them; many medium ones are the allocator's arenas. And both threads now count the piece buffers they let go of against what the collector actually took back — shared memory lives until BOTH isolates release it, so one side answers only half the question. A gap that widens means a reference of ours outlives the piece; a gap that does not means whatever grows is below us. Field 2026-08-31: 700 MB unaccounted, growing 5.4 MB a minute while the piece store's own memory fell.

## 0.56.8

- **New**: Pulls proxy 2.64.8. A backward restart of the encoder now says what it costs. Nothing already written is lost — every run keeps its own directory and the session serves the union of all of them — so the price is work about to be done twice, because the new run walks forward through segments the old one had already finished, and it is the viewer in front, who has nothing produced ahead of them until the run gets back to where it already was. The line says how far back it went, how many of those segments are already on disk, and the running totals for the session. It is the reading roadmap item 64 needs before a session is allowed more than one concurrent encode run.

## 0.56.7

- **Fix**: Pulls proxy 2.64.7. A file opened at a position starts its SOUND at that position. The audio rendition is a session of its own and its start was worked out as the picture's read head less the buffer the viewer reports holding; at a cold open there is no report, and the fallback subtracted the whole 120 s look-ahead from a buffer that does not exist. Field 2026-08-31: a page opened at 588 s started its sound at 460 s, and the segment the viewer needed took 38.8 s to appear against the picture's 8.4 s.
- **Fix**: The first encode run is positioned from the position the viewer asked for, not from the figure rounded to ten seconds for the session key. 588 s became 590 s, which falls in the segment AFTER the one the viewer is inside, so the player asked for a segment behind the run and the run was restarted onto it.

## 0.56.6

- **New**: Pulls proxy 2.64.6. The torrent worker reads its own memory once a second and writes a line when the heap has moved, and it writes a heap snapshot of ITS OWN isolate into `/data` on every new high-water above 400 MB, three kept. That isolate has been killed three times in two days for reaching its 2240 MB ceiling — 2026-08-30 14:00 and 23:19, 2026-08-31 13:27 — each time with the whole rise fitting inside one sixty-second gap, and every snapshot the proxy has ever written was of the main isolate, whose heap is 26 MB. The line now also says the ceiling beside the heap.
- **Fix**: Eight defects in the piece store's reservation accounting, found by reading it after that third death: a slot lost on any failure between claiming and filling it, a failure path that could take back another claim's reservation, a read that retried every 50 ms for ever instead of failing, a wait that allocated per pending spill, a close that stranded whoever was waiting, a stale disk copy resurrected by its own in-flight write, an unhandled rejection that could end the torrent thread, and an LRU whose capacity never followed the allowance.

## 0.56.5

- **Fix**: Pulls proxy 2.64.5. A rung measured at 0.007x (4K HEVC on CM4, field 2026-08-31) is no longer kept just because it is on screen — the `playingHeight` exemption now follows the measured check, and `ownHeight` is kept only for a copied source. A 4K HEVC transcode at 0.007x with 0.04s buffered no longer stalls the viewer with no way to downgrade; the offer can become empty and the viewer gets an error instead of an endless spinner.

## 0.56.4

- **Fix**: Pulls proxy 2.64.4. Piece store now holds one `SharedArrayBuffer` per resident piece instead of one growable pool that never shrunk — `committed` is `resident`, memory is returned on eviction and when the allowance is lowered, and the fragment path carries the piece's own buffer. The pool that leaked 650 MB is gone.

## 0.56.3

- **Fix**: Pulls proxy 2.64.3. Memory diagnostics — high-water heap snapshot and high-rss warning.

## 0.56.2

- **Fix**: Pulls proxy 2.64.2. Log torrent removal reason, refcount and caller for idle/disk-cap eviction; log stats getTorrent failures.

## 0.56.1

- **Fix**: Pulls proxy 2.64.1. Spec compliance: `FlagDefault` zero-length as default 1 (was reading as false), `S_TEXT/WEBVTT` as text.

## 0.56.0

- **New**: Pulls proxy 2.64.0. Container/track domain per spec (RFC 9559 / ISO 14496-12): `Container` → `MatroskaContainer`/`Mp4Container`/`AviContainer`, `ContainerTrack` → `VideoTrack`/`AudioTrack`/`SubtitleTrack` → `Text`/`Image`, `FlagForced` only on subtitles, `LanguageBCP47` MUST, `track_enabled`/`alternate_group`/`elng`; orchestrators (`ContainerOrchestrator`, `SubtitleOrchestrator`) and controllers (`Playback/Subtitle`) per roadmap 62. Also pulls 2.63.1 fix: reader window follows live memory allowance, not birth reservation.

## 0.55.0

- **Fix**: Pulls proxy 2.63.0. The memory this addon holds is now measurable rather than guessed: the piece store says what it has taken as well as what it holds, the torrent worker reports its own memory (where the piece pool actually lives), and the line carries what neither counter claims plus the free space on disk. A store's allowance follows the machine instead of being fixed when it was created, so it stops growing into memory the host no longer has. Also: a busy connection filling the viewer's buffer is no longer mistaken for a frozen one, which had been spending three-minute packet captures on healthy sessions.

## 0.54.0

- **New**: Pulls proxy 2.62.0. Each session now says what its cushion actually is — seconds of film ready ahead of the earliest viewer's picture, the megabytes that represents off the swarm, and how much the browsers say they hold. Deepening the browser's buffer in 0.53.0 had no reading behind it; this is that reading, and it is also what says what the deeper buffer costs the person hosting the proxy.

## 0.53.0

- **New**: Pulls proxy 2.61.0. The proxy now tells the browser how far ahead of the viewer it keeps video produced, so the player can hold all of it instead of the thirty seconds it kept while two minutes stood ready on disk. A held segment request is judged against the same figure, measured on the session's own cut grid, and against the position of the viewer who actually made it — one session serves everyone watching a copied picture, and a seek by the viewer in front used to release the requests being held for the viewer behind them.

## 0.52.0

- **Fix**: Pulls proxy 2.60.0. One film watched by more than one person is one session on this machine when the picture is copied, and the proxy now tells those viewers apart. A separately published audio track starts where the EARLIEST of them is, instead of at a position worked out by subtracting one viewer's buffer from another viewer's furthest request; the quality budget acts on the slowest link and the emptiest buffer among them rather than on whoever reported last; and a viewer who has left stops deciding for the ones still here.

## 0.51.3

- **Fix**: Pulls proxy 2.59.3. Two quality rungs that the machine encodes at the same size now share one encoder instead of starting one each. On a CM4 that had left three ffmpeg processes producing one identical picture, every rung above 240p running at 0.04x of realtime, and the viewer watching a slideshow that ended in a spinner.

## 0.51.2

- **New**: Pulls proxy 2.59.2. A failed encode run now says what it asked the source for and what the source actually holds, beside the command it ran. Three sessions died on 2026-08-26 with nothing in the log but `ffmpeg exited with code 255`, which does not separate "we asked for a track this file does not have" from any other refusal.

## 0.51.1

- **Fix**: Pulls proxy 2.59.1. The usrsctp state script crashed instead of reporting "no association found" when no viewer was connected yet — found by hand-verifying 2.59.0 against the live process right after this addon's own rollout.

## 0.51.0

- **New**: Pulls proxy 2.59.0. A wedge (roadmap item 11) is now declared, and its evidence gathered, even on small-message traffic that never fills the send queue — the only trigger that existed required a nonzero queue, and the last real episode never set it. usrsctp's own association state (peer receive window, pending data, retransmission timeout) can now be read on either wedge trigger, automatically, via a bundled `gdb` script — no rebuild needed since `node_datachannel.node` ships unstripped. `gdb` is now installed in the image permanently rather than by hand after every rebuild.

## 0.50.3

- **Fix**: Pulls proxy 2.58.3. The kernel killed the addon on 2026-08-27 for memory — `exit code 137`, no core dump, 2.4 GB resident on a host with under two free — and the log had never recorded a single figure about memory. It now says once a minute what the process holds and what the machine has left, the torrent stores share one budget across the process instead of each taking its own, and that budget comes from the kernel's `MemAvailable` rather than from a number that on Linux means something else.

## 0.50.2

- **Fix**: Pulls proxy 2.58.2 and `utp-native` 2.5.3-ttv.8. The structs holding libuv's UDP socket, timer and send requests are no longer JavaScript buffers: the module allocates and frees them itself, once libuv has finished with them. That is what ends the crash family rather than patching another route into it — every one of the nine deaths was libuv pointing into memory the garbage collector had taken back. 77 tests pass on the target architecture and sixty create-and-destroy cycles leave memory flat.

## 0.50.1

- **Fix**: Pulls proxy 2.58.1 and `utp-native` 2.5.3-ttv.7. ttv.6 registered an environment cleanup hook for every uTP context and never removed it once a context closed normally, so the hook outlived the memory it pointed at and, at teardown, called `uv_close` on handles that had already gone. That put a dead handle into libuv's closing machinery, and the process died later when a healthy handle was unlinked beside it. Read from the core dump down to the faulting instruction and by walking the loop's handle queue until a node could not be read.

## 0.50.0

- **Fix**: Pulls proxy 2.58.0 and `utp-native` 2.5.3-ttv.6. The ninth core dump of this family named a fault with no uTP frame in it at all — the torrent worker thread ending, and node faulting as it closed what was left on that thread's event loop. The module registers two libuv handles that are fields of a struct living inside a JavaScript buffer, so the loop holds pointers into memory the collector owns until those handles are closed, and the thread ending takes the memory first. A cleanup hook now closes them before the environment goes, every callback checks a plain flag before touching the JavaScript bridge, and a strong reference holds the buffer until both handles are closed. The thread ending is still a lost session, but it is no longer a dead process.
- **New**: The worker says why it is ending, from inside itself and before anything is torn down, with a list of what still held its loop open. The line the parent had for this never printed once — the crash arrives during the thread's own teardown, ahead of the event the parent waits for.

## 0.49.4

- **Fix**: Pulls proxy 2.57.1 — the flags a Matroska file states about its own subtitle tracks are read instead of guessed from the words a releaser typed into a track name: `FlagForced` (a track of signs and foreign speech, not of dialogue), `FlagHearingImpaired`, `FlagEnabled` (a track marked unusable is no longer offered, though it keeps its place in the numbering because ffmpeg keeps it), and `LanguageBCP47`, which the specification says MUST be preferred over the three-letter code.

## 0.49.3

- **Fix**: Pulls proxy 2.57.0. 0.49.2 was built before 2.56.0 reached npm, so it reinstalled the previous proxy from the cached layer; this bump is what actually delivers both releases.
- **New**: The packet witness now records the seconds BEFORE a delivery freeze rather than only after one. A size-bounded tcpdump ring runs while a viewer is being served and its files are kept the moment a wedge is declared, and the capture that follows runs for three retransmission timeouts, long enough for silence on the wire to mean something. The wedge itself is declared when it is certain — the queue has not fallen, the accepted-byte counter has not moved for longer than this connection's own arithmetic allows, and the peer is still sending — instead of after a fixed half minute, which is why neither field capture holds the onset.

## 0.49.2

- **Fix**: Pulls proxy 2.56.0 — subtitle tracks are numbered as ffmpeg numbers them, so a file carrying a picture-based track no longer sends cues to a track the browser does not know or falls back to reading the whole film for them; one walk of a file at a time, instead of a fresh one on every verified piece racing the timer over the same clusters; and every push says what film time its cues cover and how much of the file has been walked.

## 0.49.1

- **Fix**: Pulls proxy 2.55.14 and `utp-native` 2.5.3-ttv.5. The fork carries ten defects found by reading the whole binding after the seventh crash of this family named its frame: three callbacks read the connection pointer without checking it exists (and a socket refused before it was accepted carries none, while its destructor announces itself regardless — the ttv.4 patch created one such path itself); the read callback copied at an accumulated offset without consulting the buffer length, writing past the end, which is the best candidate for six deaths inside libuv bookkeeping between 2026-08-18 and 08-25; a connection could be destroyed twice; and the callback macro checked none of the napi results it used. It also gains IPv6, which the module never had, and a resolver that no longer hands it addresses it cannot use. 77 checks pass on the target, two of which used to fail or hang.
- **Chore**: Every trace of the `SCTP_DEBUG` build is gone — the `sctp_debug` option, its schema entry and the `--sctp-debug` argument in `run.sh`. Rebuilding libdatachannel from source is not something this image does; the option pointed at a flag that only ever did anything in such a build, and the proxy no longer carries the flag either (2.55.14). The delivery probes answer the question that build was wanted for, from both ends, with no native compilation.

## 0.49.0

- **New**: Pulls proxy 2.55.13 — numbered delivery probes with a computed verdict (`flowing` / `stream-stuck` / `association-stopped` / `reverse-direction-gone`), the far end's own account of itself logged as `[dc-far]`, and `GET /api/delivery-sink` behind the new `delivery_sink` option (off by default). The sink is what gives the delivery freeze something to freeze: it takes hundreds of megabytes through the real transport before the state appears at all, and a film cannot be made to deliver on demand.
- **Change**: The `SCTP_DEBUG=ON` source build of libdatachannel is gone from the image. It was there to separate the two remaining causes inside SCTP, and it cost a broken image once already (0.48.5, fmp4 init `500`) while adding several minutes of native compilation to every rebuild. The probes read the same freeze from both ends without touching the native build — a probe on an unordered, no-retransmit channel passes head-of-line blocking in another stream but not a closed receive window, which is the distinction the verbose stream was wanted for. `sctp_debug` stays as an option for an image deliberately built with the flag again, and is no longer forced on at startup.

## 0.48.8

- Restore `SCTP_DEBUG=ON` diagnostic build with libdatachannel pinned to the exact commit of v0.24.2 (FetchContent was pulling a moving tag — the 0.48.4→0.48.5 behavior change came from an unpinned rebuild). The `init 500` regression is therefore not expected to return with the pin in place.

## 0.48.7

- Make `poison_heap` opt-in again (was always-on in 0.48.6 and crashed at startup on this host). `sctp_debug` stays always-on in this diagnostic series.

## 0.48.6

- Diagnostic build: logging stays on without manual steps until removed. Core dumps allowed (`ulimit -c unlimited`), freed pages become inaccessible and any later access crashes immediately at the offending instruction (core dump names the module), and SCTP verbose stream (`usrsctp:`) is always on. No config toggle needed for this build.

## 0.48.5

- Revert diagnostic `SCTP_DEBUG` build (init segment `500` on `video=copy` — the patched `node-datachannel` rebuild broke fmp4 init generation). Proxy stays at 2.55.12 (initLogger fix kept); `poison_heap` stays available as a file in the repo but is not built into this image.

## 0.48.4

- Pulls proxy 2.55.12. Fixes the `sctp_debug` wiring (one-arg call always threw `Function expected` — `node-datachannel` 0.32.x needs the callback) so verbose SCTP lines actually appear. Poison heap stays available via `poison_heap` toggle (fixed in 0.48.3, disabled by default for the previous run).

## 0.48.3

- Fix `poisonmalloc.c` (protected the backing page instead of the payload page — immediate SEGV at startup) and fix `bin/cli.js` `--sctp-debug` binding (called the method without its `this`). Bumps proxy to 2.55.11.

## 0.48.2

- Fix `run.sh` (bad `bashio::config.equals` argument order — diagnostic flags never fired).

## 0.48.1

- Fix build (missing `errno.h` for `poisonmalloc.c`).

## 0.48.0

- Pulls proxy 2.55.10. Diagnostic build: node-datachannel is rebuilt with `SCTP_DEBUG=ON` and the proxy exposes `--sctp-debug` — when enabled, the browser-side SACK stream (`a_rwnd`, gap reports, retransmissions) is printed as `usrsctp:` lines into the log, which is what separates the two remaining hypotheses for the delivery-side freeze of 2026-08-24/25. The flag is off by default; toggle it in the addon configuration (`sctp_debug`) for the reproduction run only — the log volume is high.
- Diagnostic: the image now also builds `poisonmalloc.so` and toggles it with `poison_heap`. When on, freed pages become `PROT_NONE` in a bounded quarantine so any use-after-free faults immediately at the offending instruction and the core dump names the guilty module. Off by default; same config-toggle pattern.

## 0.47.0

- Pulls proxy 2.55.9. When a viewer's send queue stays wedged for over 30 s, the proxy now records the wire itself: a bounded tcpdump (128-byte packet headers, 4 × 30 s ring, 120 s total, one capture at a time) lands beside the core dumps, so the rare one-way transmit death of 2026-08-24 carries its own evidence instead of 88 minutes of counters that all say success.
- The image now ships tcpdump — without it the witness could only ever log its own absence.

## 0.46.0

- Pulls proxy 2.55.8. Fixes the crash that killed the proxy twice in one evening (2026-08-22): with everything quiet — viewer paused or gone, upload disabled, idle peer connections closed — the torrent worker thread's event loop drained and the thread ended by itself; its teardown then touched already-freed memory (SIGSEGV during worker shutdown) and took the whole process down instantly. The worker now keeps one interval accounted for, so the loop never drains while the process lives.

## 0.45.0

- Pulls proxy 2.55.7. Fixes subtitle push outright: the subscription was recorded under the browser's registry key while the pool published under its own infohash-based key, so a push never found a subscriber for any torrent since the feature shipped. Now resolved through `sourceRegistry` at the point of subscription.

## 0.44.0

- Pulls proxy 2.55.6. Diagnostic only: every step of the subtitle push chain now logs on success (subscribe, cues found, cues sent), needed because a field report of cues still not appearing at once had no evidence in the log to work from either way. Needs server 0.16.3 or later for the matching browser-side log.

## 0.43.0

- Pulls proxy 2.55.5. Subtitle cues are now PUSHED to the browser over the WebRTC data channel the moment the proxy reads them, instead of being fetched on a timer — rides the existing `proxy-control` channel, never the one carrying segment bytes. Needs server 0.16.2 or later, which receives the push and creates every declared track's element up front instead of one at a time behind a fetch.

## 0.42.0

- Pulls proxy 2.55.4. Subtitle cues now warm on the piece-verified event instead of a 3 s poll — a cue is walked the instant its cluster's piece becomes readable, off WebTorrent's own `verified` signal, so nothing waits on a schedule to be picked up.

## 0.41.0

- Pulls proxy 2.55.3. A subtitle track's cues are now walked ahead of being asked for — a periodic pass in the torrent worker keeps every actively-read file's cues caught up with what has downloaded, instead of only walking clusters inside a browser's first request for a track (which paid for the whole backlog serially, on a film well into playback).

## 0.40.0

- Pulls proxy 2.55.2. The decode benchmark's pipe sanity log now compares figures over the same window instead of the whole run, and only flags a divergence when the two are actually more than 1.5x apart, instead of printing an editorial line unconditionally on every reading.

## 0.39.0

- Pulls proxy 2.55.1. A magnet URI and a `.torrent` file for the same content are now keyed by their shared infohash instead of by a hash of the request bytes, so the two share one swarm and one cache from the first request instead of colliding as `WebTorrent client error: Cannot add duplicate torrent`. Also logs the decode benchmark's actual pipe throughput beside what it needed, as a sanity check on the reading.

## 0.38.0

- Pulls proxy 2.55.0. The decode calibration was measuring the loop rather than the decode — a clip restart costs the decoder 0.03-0.12 s and a five-second clip at 55x restarts eleven times a second — so every reading this host takes of itself was 25-33 % low, and biased differently per clip. The clip is now fed to the decoder as one continuous stream, the readings are ordered the way decoding is, and the whole startup calibration is shorter than before.

## 0.37.0

- Pulls proxy 2.54.0. The automatic quality step no longer rewrites the SIZE of the picture underneath a session the browser is already decoding — a change of resolution is a change of variant now, and the proxy asks the player to move rather than changing what it is producing. Also brings the step back UP, which has never existed, the quality step on the copy path, and a bitrate ceiling taken from the viewer's own measured link.

## 0.36.1

- Pulls proxy 2.53.0, which carries the pin to `@torrent-tv/utp-native` 2.5.3-ttv.4. The 0.36.0 build failed: the version is stated in two places — the proxy's own `overrides` and this Dockerfile — and moving only the second made npm refuse the install as a conflicting override.

## 0.36.0

- Builds `@torrent-tv/utp-native` 2.5.3-ttv.4, which stops the proxy dying every few hours. `on_utp_accept` asked JavaScript for the buffer for the next connection and then handed the returned value to napi without checking that anything had been returned — the value was an uninitialised local, and the macro that fills it inspects only one kind of failure and carries on regardless. Two core dumps on 2026-08-21, at 16:49 and 19:50, both on the thread that owns the uTP socket and both with the same top frames; the bottom of that stack is the ordinary event loop, so it is not the shutdown race fixed earlier. Every live session on the proxy died with it, each time.

## 0.35.0

- Pulls proxy 2.52.0: reopening a film at a position no longer loses the sound. The picture went to the right place and the soundtrack started re-encoding from the beginning of the file, so the player waited 45 seconds for audio that was 52 minutes away and was told the proxy had sent no video.
- Pulls proxy 2.52.0: the log line about a torrent's swarm now says how many peers are connected, how many the client knows but has not connected to, and what the tracker reported — enough to tell "nobody was offered" from "several were offered and we reached none". Before it printed a connection count beside a field that was always a question mark.
- Pulls proxy 2.52.0: the wait for the first connected peer is measured and reported. One file waited 4 min 17 s for its first peer while the tracker was reporting five seeders, and nothing counted or showed it.
- Pulls proxy 2.52.0: one torrent is logged under one identifier — its infohash — instead of three different hashes across neighbouring lines.

## 0.34.0

- Pulls proxy 2.51.0: seeking no longer makes the picture drift behind its own numbering. A run was positioned on one boundary table and cut on another, so every restart carried the distance between them into all of its cuts — and since that distance accumulates, the drift grew with each seek: two segments after the first, four after the next. Past a certain point the player's buffer stops extending at all and the picture jumps forward on its own.
- Pulls proxy 2.51.0: the DHT is given entry points that answer. Two of the three the library ships are dead and the third could not be reached by name on a host with IPv6, so the routing table stayed empty and a torrent had only its trackers to find peers with — one file waited 4 min 17 s for its first peer while the tracker was reporting five seeders. An empty routing table is now reported instead of passing unnoticed.
- Pulls proxy 2.51.0: a stalling player is answered rather than left talking to its own console, and a run says where it was positioned and how far the two boundary tables have moved apart.

## 0.33.9

- Pulls proxy 2.50.0: seeking in an AVI no longer lands a keyframe early. AVI names its keyframes by frame number, and the resulting time sits up to a frame away from the real one — enough for a seek to fall just short and go back to the previous keyframe.
- Pulls proxy 2.50.0: a file whose container carries no keyframe index at all — MPEG-TS, for instance — is re-encoded instead of copied. A copied picture can only be cut at the source's own keyframes, and without an index nobody knows where they are, so the playlist was describing cuts the file does not have.

## 0.33.8

- Pulls proxy 2.49.0: the addon should stop dying in the middle of a film. A core dump named the fault — the thread that owns the torrent client was being torn down while a datagram was still arriving, and the datagram walked into memory that had just been freed. The thread is now allowed to finish by itself instead of being stopped under itself, and a thread that ends is written to the log rather than passing in silence.
- Pulls proxy 2.49.0: crash dumps are capped at the newest two. Each is over four gigabytes and four of them had nearly filled the disk.

## 0.33.7

- Pulls proxy 2.48.0: after a seek, the picture now arrives where the player expects it. ffmpeg moves an input seek back by 130 ms on containers that seek in decode order, so a copied picture began one keyframe earlier than asked and every segment of that run was cut two seconds from where its playlist said — four times what a player can bridge, after which the same fragment is fetched again and again until the film dies. Confirmed against ffmpeg's own source and measured on both Matroska and MP4.

## 0.33.6

- Pulls proxy 2.47.0: the addon no longer disappears from the registry for five seconds every hour and forty minutes. Something upstream was closing the connection after exactly 100 min 15 s, and a viewer arriving in the gap was told there was no proxy; the connection is now replaced at ninety minutes, with the replacement taking over before the old one closes.

## 0.33.5

- Pulls proxy 2.46.0: the quality menu stops over-promising on the files that actually need re-encoding. What a video costs to decode is now measured separately for HEVC and for 10-bit HEVC instead of being guessed from H.264 — and a video that has to be re-encoded is, by definition, one the browser could not play, so it is usually one of those.

## 0.33.4

- Pulls proxy 2.45.0: a fragment now lands where the player was told to expect it. Segments were being cut on a grid the proxy had since corrected while the player still held the original one — 119 of 125 of them arriving a uniform 2.002 s early on the field film, four times what a player can bridge, after which the same fragment is fetched again and again.
- Pulls proxy 2.45.0: which subtitle track a file asks for is read out of the file rather than out of ffmpeg's description of it, so a file that marked no track is no longer indistinguishable from one that marked every track.

## 0.33.3

- Pulls proxy 2.44.0: subtitles stop appearing at moments that have nothing to do with the picture. A track asked for what it lacked by film time, while cues come out of whichever clusters are downloaded and those do not arrive in order — so one answer carrying a line from far ahead made everything behind it unreachable for the rest of the session.
- Pulls proxy 2.44.0: reading a subtitle track costs a fraction of what it did. One walk over the container now fills every track instead of one walk per track, and the same cluster bytes were being fetched and parsed five times on the field film.

## 0.33.2

- Pulls proxy 2.43.2 and builds `@torrent-tv/utp-native@2.5.3-ttv.3`: the proxy no longer dies without a word in the middle of a film. A failed UDP read left the uTP library parsing a sender address that never existed, killing the process — three times in two days.
- Pulls proxy 2.43.2: a soundtrack no longer moves the grid the picture is cut on. The two readings disagreed by 1.951 s on the field file and overwrote each other for as long as it played.

## 0.33.1

- Pulls proxy 2.43.1: a subtitle request says where the browser's copy ends and gets back only the cues past it, instead of the whole track — 76 KB a time on the field file — every few seconds.

## 0.33.0

- Pulls proxy 2.43.0: an MP4's text subtitles are read from its sample table, which states every cue's own byte range — so a cue costs its own few dozen bytes instead of the cluster around it, and still never costs a request. `tx3g` and `wvtt` are decoded; TTML is left out rather than half-shown.
## 0.32.0

- Pulls proxy 2.42.0: embedded text subtitles are read out of the clusters the film is already downloading instead of being extracted with ffmpeg. One track used to cost a full extra copy of the film and 752 seconds — the browser gave up after 60 — because a subtitle stream is sparse and the demuxer walks the whole container whatever range is asked of it. The blocks sit in clusters the viewer is downloading anyway, so the cues now cost no extra bytes and are ready before playback reaches them.
## 0.31.3

- Pulls proxy 2.40.2 and `utp-native` 2.5.3-ttv.2: the second place that read a callback result nobody wrote is guarded. With the first fix in the loading path the process still died twice within an hour, and the stack named `on_utp_accept` instead of `on_utp_read`.

## 0.31.2

- The check added in 0.31.1 also matched an intermediate build artefact under `obj.target`, which has no manifest three levels up, so it failed a build that was in fact correct. It now looks only at `build/Release/utp_native.node`.

## 0.31.1

- Pulls proxy 2.40.1: the patched `utp-native` finally reaches WebTorrent's own nested copy, which is the one it was loading all along — the crash of 21:03 names that path in its stack, and so did every earlier one. The image now deletes nested copies after the install and fails the build if any surviving binary is not ours.
## 0.31.0

- Pulls proxy 2.40.0: what a reader wants is claimed in four bands of decreasing urgency instead of one, so what the viewer reaches in seconds is no longer indistinguishable from the rest of the film. The widths of the lead bands come from what has been measured about the file on its swarm — the worst interruption met, and the surplus the swarm delivers between interruptions. Which way a read claims is decided per read, and the log compares the two directly, so the next sessions say by number whether it helps.
## 0.30.1

- Pulls proxy 2.39.1: the block duplication of 2.39.0 is removed. Measured against what a viewer feels — the seconds the picture stands still — it never paid: nothing to improve on a well-seeded film, and with the download capped just above the film's rate, which is what a home line is, it made the picture stop for 28.8 s against 12.8 s. Under one shared budget a duplicate spends the very bytes it is trying to go around.
## 0.30.0

- Pulls proxy 2.39.0: when a reader is stuck on a piece and every block of it is already spoken for, those last blocks are asked of a second, faster peer as well. Measured against itself on one film, six pairs across two pacing rates: the median wait for a piece fell in all six, by 6 to 68 per cent. Costs about 3 per cent extra traffic, and only while a reader is actually blocked.

## 0.29.1

- Pulls proxy 2.38.1: when a blocked piece cannot be steered onto a faster peer, the wait line now says what is holding it — how many blocks are still missing and on which wires they sit, slowest first. Measured before anything is built on it, because duplicating those blocks costs traffic and the numbers decide whether it is worth doing at all.

## 0.29.0

- Pulls proxy 2.38.0: an MP4's keyframe times are read as composition times, on the track the handler names. Measured against ffmpeg itself: a release with a composition offset and no edit list was 62.1 ms early on every keyframe, and one where the two cancel is unchanged and exact.

## 0.28.1

- Pulls proxy 2.37.1: a seek's own cost no longer counts against the quality offer. The wait on the first piece after a jump measures the move, not the swarm, and letting it in collapsed a five-rung menu to one 131 ms after a seek.

## 0.28.0

- Pulls proxy 2.37.0: the segment a viewer seeks TO is no longer refused as stale — on 2026-08-18 that refusal froze a session for 149 seconds while the player re-fetched two audio segments 1473 times. The proxy also writes its log to `/data/proxy.log` now (rotated at 32 MB, one previous turn kept): the container is recreated by the watchdog on every crash and by every update, and each time it took the lines explaining the failure with it.

## 0.27.4

- Builds uTP from our own source instead of the published library. `utp-native` reads an uninitialised `napi_value` after `napi_make_callback` and hands it to `napi_get_buffer_info`, which dereferences it inside V8: the proxy died thirteen times in three days with SIGSEGV in `on_utp_read`, always on incoming data, and the watchdog recreated the container each time — which is what a viewer sees as "Connection to the proxy was lost". The addon now installs `@torrent-tv/utp-native@2.5.3-ttv.1` under the name WebTorrent imports and compiles it there, as it already had to (no musl prebuilds exist). Temporary: when https://github.com/mafintosh/utp-native/pull/58 is released, the install line goes and the registry version is used again.

## 0.27.3

- The proxy now leaves a diagnostic report behind when it dies. The supervisor's watchdog restarted this addon twelve times between 2026-08-16 and 2026-08-18 — eleven times on exit code 139 (SIGSEGV) and once on 134 (SIGABRT) — and every one of those is a viewer's "Connection to the proxy was lost". The crash is in native code, so nothing appears in the proxy's own log; Node's report carries the native frames and is written to /data, which survives the container being recreated.
## 0.27.2

- **Fix**: Peer discovery no longer starves behind name resolution. Node resolves host names on a four-thread pool by default, so a torrent announcing to ten trackers at once resolves four and queues the rest — and a tracker that no longer exists holds its thread for the resolver's whole ten-second timeout while every announce behind it misses its own deadline. Measured in this container: those ten names took 7.58 s as a burst against 27-42 ms each with a larger pool, and the film — 517 seeders on a tracker that answers in 50 ms — spent eleven minutes with zero peers. The run script now states the pool size before the proxy starts.
- Pulls proxy 2.36.2: a live session no longer answers 404 to the master playlist it just published, which stopped playback outright on 2026-08-18, and the same pool fix ships inside the proxy for other ways of running it.
## 0.27.1

- Pulls proxy 2.36.1: a copied picture is cut at its own keyframes again. The container's table names a track per entry, and both field films index their subtitle tracks as well — 4669 and 5007 extra entries — which were being read as though they were keyframes, so every cut asked for at one of them landed at the next real keyframe instead: 2.002 s late on one film, a median of 6.3 s and up to 21 s on the other.
## 0.27.0

- Pulls proxy 2.36.0: the torrent is charged for the megabytes it is actually measured to be moving, at a price that no longer contains the process's own idle draw (which made the same host report 145 ms/MB over 8.7 MB and 23 ms/MB over 54 MB), and a quality step is judged against the speed this file's own supply demands instead of a hand-chosen 1.5.
## 0.26.0

- Pulls proxy 2.35.0: what a second encode costs this machine is measured at startup instead of assumed — on this host the same work is 2.6x dearer with one encoder beside it — and the quality offer uses that instead of adding independent prices.

## 0.25.0

- Pulls proxy 2.34.0: the proxy measures the smallest buffer at which no interruption reaches the viewer and tells the browser, which stops waiting for a hand-chosen twenty-five seconds.

## 0.24.0

- Pulls proxy 2.33.0: a quality step is judged on the machine it will actually run on — the prediction is multiplied by the share of the host that is free — and each running step reports what its prediction was worth against what it measured.

## 0.23.0

- Pulls proxy 2.32.0: the decode cost is fitted from six calibration clips instead of three, so a term the measurements cannot determine is refused rather than published as a zero. The startup measurement takes longer for it, and the log now says which terms the clips determined.

## 0.22.0

- Pulls proxy 2.31.0: the encoder run's status is one thing in one place — the two hand-maintained strings that could disagree are now computed from the run's own state.

## 0.21.2

- Pulls proxy 2.30.2: the cut-time shift is reverted — the field showed it moved the picture's cuts off the source's own keyframes — and the steering count in the log compares like with like.

## 0.21.1

- Pulls proxy 2.30.1: a seek is no longer undone a second later by a request the player issued before it.

## 0.21.0

- Pulls proxy 2.30.0: the log now states, per file, the speed a quality step must sustain to survive this swarm and the smallest buffer that hides an interruption — both computed from the reader's own waits rather than chosen by hand.

## 0.21.0

- Pulls proxy 2.29.0: a piece the reader is waiting for is requested from the fastest peers that have it, instead of being left with whichever peer happened to take it. The swarm was delivering five times what the film consumed and the reader still waited 47 times in two minutes.

## 0.20.0

- Pulls proxy 2.28.0: the playlist and the media agree again. On files whose first timestamp is not zero, the copied picture was cut that far from where it was asked, disagreed with the soundtrack about every boundary, and the two corrected each other all session — which is what made the player refetch fragments endlessly.

## 0.19.0

- Pulls proxy 2.27.0: picture and sound begin a run at the same instant. The picture can only start at a real keyframe and moves forward to it; the soundtrack has none and starts exactly where asked — up to three seconds apart after every seek. The soundtrack now follows the picture to the moment the picture really began.

## 0.18.0

- Pulls proxy 2.26.0: the log now reports, per file, how far produced segments fell from the playlist and how many of them began at another keyframe the container's own table names — enough to settle whether the table is wrong or our grid is built over gaps in it.

## 0.17.1

- Pulls proxy 2.25.1: picture and sound are back in step — a segment is stamped with its own start again, as it was before the two releases that tried moving it toward the playlist.

## 0.17.0

- Pulls proxy 2.25.0: picture and sound no longer drift apart after a seek — every session of one film publishes the same timeline — and the read window grows into a lead while the reader is being made to wait.

## 0.16.0

- Pulls proxy 2.24.1: a segment is stamped where the playlist the player holds says it begins. On a file whose container keyframe index is wrong, a seek used to leave the browser refetching the same two segments for ever — 1908 times each over ten minutes, measured — with the picture frozen.

## 0.15.0

- **Fix**: Pulls proxy 2.24.0 — the quality you just chose stops disappearing from the menu. Preparing a quality in the background is a second conversion running beside the one on screen, and the box counted that second conversion against itself: the quality being prepared looked unaffordable purely because it was being prepared, so it was dropped from the menu and the picture it had already made was refused.
- **New**: The box now counts what a separate soundtrack costs it, and what a second picture costs while both are running. Both were counted as nothing, which is why it kept offering qualities it could not hold.
- **Fix**: A speed measured while the film was not downloading fast enough is no longer taken as a measurement of the box. Twenty seconds of a slow download used to make the box look ten times weaker than it is, and every quality above the one on screen was withdrawn on the download's account.
- **Fix**: The log stops repeating the same line about withheld qualities every few seconds for the whole film, which was pushing everything else out of it.

## 0.14.0

- **Fix**: Pulls proxy 2.23.0 — seeking no longer damages the box's own settings. Every jump through a film killed the running conversion, and the proxy read that killing as the conversion having FAILED: on a machine with a graphics chip that can convert video, it then switched itself to the slow software converter for good, and started an extra conversion at the position you had just left. A moment of "failed" also sat between the kill and the restart, which a request arriving right then was answered with an error.
- **Fix**: A film whose data briefly goes missing no longer costs the box its fast converter either. Losing the download says nothing about the converter, and it was being treated as a converter fault.
- **New**: The conversion's own progress through its lifetime — starting, producing, suspended, waiting for data, stopped — is now written down as a table the code follows, and every step it takes is recorded in the log. Nothing acts on it yet; it is there so the next failure names itself instead of being reconstructed by hand.

## 0.13.0

- **Fix**: Pulls proxy 2.22.0 — the quality menu stops emptying itself. Which qualities are OFFERED comes from the measurement taken at startup against known clips, which does not move; the figure learned while a film plays moves with whatever else the box is doing, and three sessions in a row it refused one more quality each time until only one was left and the menu vanished. A quality actually SEEN running too slowly, with the box to itself, is still withdrawn — that much a live reading can speak for.

## 0.12.1

- **Fix**: Pulls proxy 2.21.1 — this box no longer teaches itself impossible costs. It was measuring what copying and decoding a film cost while TWO conversions shared the machine, so each measurement came out several times too expensive, every lower quality was refused as impossible, and the quality menu disappeared along with them. A cost is now only taken from a conversion that had the box to itself.

## 0.12.0

- **Fix**: Pulls proxy 2.21.0 — the two measurements added yesterday (what copying a film costs this box, and what downloading and serving it costs) were never actually taken: both read from places that hold nothing here. They are taken now, and three faults in how they would have been read are fixed with them — most importantly that a copy's speed was being averaged over the minutes it spends deliberately paused, which made it look five times more expensive than it is.

## 0.11.0

- **Fix**: Pulls proxy 2.20.0 — a film's readers now share the memory set aside for it instead of each claiming the whole of it. On this box that is what emptied the store on 2026-08-15: with the soundtrack served separately there were two readers, a quality warm-up made three, and playback stopped outright.

## 0.10.0

- **New**: Pulls proxy 2.19.0 — the box measures what downloading and serving a film costs it, and counts that against the quality it offers. With every conversion suspended this box was still a fifth to a third busy, and none of that was being counted; a quality choice was therefore weighed against a machine that does not exist.

## 0.9.0

- **New**: Pulls proxy 2.18.0 — the box now counts what copying a film costs it, and weighs a quality choice against the work it will really be doing rather than against an empty machine. On this very box the sum comes out as the failure did: an eighth of the machine for the copy plus the rung itself is more than one second of work per second, which is why picking a lower quality used to stop playback.

## 0.8.0

- **New**: Pulls proxy 2.17.0 — the startup measurement of this box's encoder is taken on real film instead of a generated pattern, and read from ffmpeg's own progress instead of a stopwatch around it. The pattern made the box look 1.23x faster than it is, and the stopwatch hid the difference between presets behind the half-second it takes to start a program; both errors pointed the same way, at offering a quality the box cannot actually produce.
- **Fix**: A measurement that cannot be trusted is now refused rather than used — a window of no width, a position some ffmpeg builds print before they have one, a speed no machine can reach. One such reading used to become the figure every quality decision was taken from.

## 0.7.0

- **New**: Pulls proxy 2.16.0 — while a film is being converted, the log says what this box is actually doing: how much of it ffmpeg is getting, how much everything else takes, how much is spent waiting on the disk, and the CPU's clock and temperature. On this very box a quality rung ran at half the predicted speed with nothing else running, and nothing recorded said why.

## 0.6.3

- **Fix**: Pulls proxy 2.15.3 — a magnet that found no peers no longer blocks the same film from opening as a `.torrent` afterwards. Until now the failed magnet left an entry with no file list, every later attempt was answered by it, and only restarting the addon cleared it.

## 0.6.2

- **Fix**: Pulls proxy 2.15.2 — changing the audio track no longer costs a minute of spinner. The player asks the new track for its first segment before anything else, and that request was held for the full minute before failing, even though nothing could ever have produced it.

## 0.6.1

- **Fix**: Pulls proxy 2.15.1 — a magnet that never found its metadata no longer makes the same film unplayable from its own `.torrent` file. Measured on this box: one magnet with no reachable trackers left every later attempt at that infohash bound to an empty torrent, and only a restart cleared it.

## 0.6.0

- **New**: Pulls proxy 2.15.0 — the audio track is made ready before the player changes to it, so a change of language no longer shows a spinner over a stopped picture.
- **Fix**: The track a viewer leaves is stopped, and only the one being listened to is moved on a seek. On this box a viewer who had changed track once had three readers on one file, and at a seek they pinned every piece the store had; a read then ended with zero bytes and every encoder took that for the end of the file.
- **Fix**: A read waits for a piece to be released instead of failing, so a momentary shortage is a pause rather than the end of playback.

## 0.5.2

- **Fix**: Pulls proxy 2.14.3 — changing the audio track is quick again. On this box 0.5.1 left the viewer on a spinner that never ended: the track was started ahead of where the picture actually was, and the proxy then refused the segments the player kept asking for instead of correcting itself. The track now starts where the picture is, measured from what the browser reports it holds.

## 0.5.1

- **Fix**: Pulls proxy 2.14.1 — changing the audio track no longer costs twenty seconds of silence. Measured on this box: the track was changed 159 seconds in, the encoder was placed correctly, and then a repair meant for a misplaced run sent it back to the beginning of the film.

## 0.5.0

- **New**: Pulls proxy 2.14.0 — audio is published once for the file instead of being encoded again for every quality rung, and changing the audio track no longer rebuilds the stream from nothing. On a box that struggles to encode one picture, encoding the same soundtrack six times over is work spent on nothing.
- **Fix**: The quality this host serves by COPY is never taken out of the menu. On 2026-08-15 on this very box, switching down to 240p made 1080p disappear four seconds later — the rung that was actually working, and the one a stranded viewer goes back to.
- **Fix**: A rung running just under realtime now teaches this host what the file costs, so the menu corrects itself. One ran at 0.95-0.999x for three minutes here and taught nothing.

## 0.4.0

- **New**: Pulls proxy 2.13.0 — a quality rung is offered only where this host can actually produce it faster than it is watched. The budget used to price encoding alone, which is half the work, and on this very box it offered a 240p rung it then ran at 0.388-0.947x while the 1080p it replaced was copied at 7.8-8.9x: picking a lower quality was what broke playback. Decoding is now measured at startup from three bundled reference clips, and then re-measured on the file being watched, from the encoder already running on it.
- **New**: What this host has measured about itself is kept in `/data`, which the Supervisor keeps across updates. It used to live inside the container, where every addon update discarded it and the first viewer after one was shown a figure with no measurement behind it.

## 0.3.65

- **Chore**: Pulls proxy 2.12.2 — the transport heartbeat is now written once per connection with each channel's queue beside it, a channel watch ends when the transport stops knowing its session, and the recorded host timings no longer ship inside the package.

## 0.3.64

- **Fix**: A momentary network failure no longer ends the build with an error about the wrong thing. node-datachannel fetches a prebuilt binary from GitHub and falls back to building from source if that one request fails — and that fallback is broken in the tool it uses, so the build died reporting `expected first argument to be an array` when what had actually happened was `socket hang up`. The download is retried, and the resulting binary is checked for rather than assumed: without it the addon starts with no WebRTC at all.

## 0.3.63

- Pulls proxy 2.12.1: the cut grid of a copied stream is corrected from the file itself where the container index is wrong, and the quality warm-up actually works — it was being cancelled by the stream still playing, which made one switch take 43.6 s.

## 0.3.62

- Pulls proxy 2.12.0: a quality rung is prepared before the player switches to it, so the switch no longer shows a spinner; a forced resolution keeps the ladder beneath it, so a rung the host cannot encode steps down instead of freezing.

## 0.3.61

- Pulls proxy 2.11.0: a quality change is placed where the player asked for it rather than where the rung being left had read to, a segment request the running encode can never reach is repaired instead of held for ever, and quality switching now works on a copied video as well.

## 0.3.60

- Pulls proxy 2.10.0: a session that re-encodes its video now publishes a master playlist, so the viewer can change quality without playback stopping. Only one encoder runs at a time — the rung nobody is watching is stopped.

## 0.3.59

- Pulls proxy 2.9.141: a held segment now records whether the encoder has moved at all since the run started, so a wait on torrent data can be told apart from a wait on encoding.

## 0.3.58

- Pulls proxy 2.9.140: the time to produce a first segment is now derived from the startup benchmark, so the estimate is right on a machine's first run instead of resting on an assumed rate; measured medians also survive a restart.

## 0.3.57

- Pulls proxy 2.9.139: the proxy now states which tracks its output carries. It had been declaring none at all, which left the browser unable to tell a file with no video from a session that lost the track, and left the init guard accepting any header.

## 0.3.56

- Pulls proxy 2.9.138: the init segment must declare every track before it is cached, so a header written before the video was muxed can no longer leave the browser playing sound with no picture for the whole session.

## 0.3.55

- **New**: Pulls proxy 2.9.137 — a session states the track set its output will carry and sends it to the browser, so a track lost on the way is named at once instead of showing as a black picture with working sound.

## 0.3.54

- **Fix**: Pulls proxy 2.9.136 — what a complete init segment must describe is taken from the track set the proxy declares it will output, instead of assuming two tracks.

## 0.3.53

- **Fix**: Pulls proxy 2.9.135 — a session no longer plays sound with no picture at all (an init segment short of a track was cached for the whole session), and a run's first segment is no longer held while it lies finished on disk.

## 0.3.52

- **Chore**: Pull proxy 2.9.134 — the restart-delay instrument prints.

## 0.3.51

- **Chore**: Pull proxy 2.9.133 — a restarted encoder no longer waits for its predecessor to die, which is about a second off every seek.

## 0.3.50

- **Chore**: Pull proxy 2.9.132 — an encoder restart reports what each of its stages cost.

## 0.3.49

- **Chore**: Pull proxy 2.9.131 — a run's first segment is served once the encoder has passed it, which is the segment every resume and seek depends on.

## 0.3.48

- **Chore**: Pull proxy 2.9.130 — a held segment now says why it is held, so the case where a produced segment is never served names its own cause instead of running out silently.

## 0.3.47

- **Chore**: Pull proxy 2.9.129 — a held segment now says why it is held, which is the instrument for a resume that never plays.

## 0.3.46

- **Chore**: Pull proxy 2.9.128 — a read that ends because the reader left is no longer logged as a failure.

## 0.3.45

- **Chore**: Pull proxy 2.9.127 — a read that hands the file over out of order now says so, which is what the AC-3 corruption of 2026-08-09 needs in order to name its own cause.

## 0.3.44

- **Chore**: Pull proxy 2.9.126 — a read whose offset falls outside its piece pool now ends short and says so, instead of throwing an exception the process swallowed and then answering no reads at all.

## 0.3.43

- **Fix**: Pulls proxy 2.9.125 — playback works again. 0.3.42 could not play anything: the proxy served the playlist and then held every request for the first segment until the browser gave up, while the finished segments lay on disk. Update straight past 0.3.42.

## 0.3.42

- **Fix**: Pulls proxy 2.9.124 — subtitles no longer drift away from the picture. A segment was labelled with the time the playlist assigned it, and on a file whose keyframe index disagrees with its own contents that was out by four seconds; the label now comes from the segment itself.

## 0.3.41

- **Fix**: Pulls proxy 2.9.123 — 0.3.39 could delete the segment the encoder was writing, so that segment never appeared and playback stopped. Update straight past 0.3.39.

## 0.3.40

- **Fix**: Pulls proxy 2.9.122 — resuming a film now starts encoding where the viewer was. The position reached the proxy and was then ignored when the encoder was started, so a Retry after a proxy restart spent 45 s transcoding the opening credits and gave up.

## 0.3.39

- **Fix**: Pulls proxy 2.9.121 — a seek no longer hangs on a segment that a terminated run left short of a track. Such a file looks complete, so it was served, and the player could not use it.

## 0.3.38

- **Fix**: Pulls proxy 2.9.120 — the rest of the film is fetched while the viewer needs nothing, instead of the link sitting idle whenever the encoder is held back; the stall warning stops firing when a download of zero is the correct state; and the progress report carries the two timings the browser needs to estimate a wait after a seek.

## 0.3.37

- **Fix**: Pulls proxy 2.9.119 — the quality menu can finally say what automatic quality has settled on. The height was reported as zero whenever the video was copied, which is most sessions, so the menu read a bare "Auto".

## 0.3.36

- **New**: Pulls proxy 2.9.118 — when the encoder is held back, the log now says what that decision was taken on, which is what a stalled download after a seek could not be explained without.

## 0.3.35

- **Fix**: Pulls proxy 2.9.117 — seeking backwards after seeking forwards no longer freezes playback. The encoder was suspended for being "ahead of the viewer" on the strength of segments the viewer had left behind, 136 ms after starting and before it had made anything.

## 0.3.34

- **New**: Pulls proxy 2.9.116 — a progress report says which height is being produced, so the player can show what automatic quality has settled on instead of just "Auto".

## 0.3.33

- **Chore**: Pulls proxy 2.9.115 — a diagnostic that could flood the addon log after a seek now reports the start and the end of what it observes, with how long it lasted.

## 0.3.32

- **Fix**: Pulls proxy 2.9.114 — seeking no longer destroys the torrent. Each seek left one piece of memory permanently reserved, and when the store ran out it reported the exhaustion to the torrent client, which closed the store and dropped the torrent; playback then failed with "file not found" until the addon was restarted.

## 0.3.31

- **New**: Pulls proxy 2.9.113 — the transport's counters are recorded continuously, so the next time data is accepted for sending and never arrives, the log says whether it left the machine.

## 0.3.30

- **New**: Pulls proxy 2.9.112 — when the data a session is playing goes away, the proxy now downloads it again and carries on, instead of reporting a failure the viewer could do nothing about.

## 0.3.29

- **Fix**: Pulls proxy 2.9.111 — the film being watched can no longer be deleted while a seek is in progress, and the encoder is no longer held back on the strength of a position ffmpeg reported but had not reached. Both were measured on 2026-08-06: six gigabytes evicted mid-seek, and a segment held for 45.7 s because nobody was producing it.

## 0.3.28

- **Fix**: Pulls proxy 2.9.110 — a torrent the proxy had cleaned up as idle was still being handed to the player, which killed every later attempt at that same source until the addon was restarted. Two sessions in a row failed this way; the source is now added again instead.
- **Chore**: The build now checks that the base image supplies Node 24, the LTS line the proxy is tested on, and fails with a clear message if it ever drifts off it.

## 0.3.27

- **Chore**: Pulls proxy 2.9.109 — a transcode session outlives a vanished browser by thirty minutes instead of ten, so a viewer who closed the tab or lost the network comes back to a warm encoder. An open tab never consumes this at all: it re-asserts itself every 30 s.

## 0.3.26

- **New**: Pulls proxy 2.9.108 — when a data channel stops delivering while still accepting requests, the log now names the cause instead of only showing that bytes are piling up: it reports what the transport itself sent and received, the round-trip time and the path in use, every second for as long as it lasts.

## 0.3.25

- **Fix**: Pulls proxy 2.9.107 — 0.3.24 could not play anything: every playback plan failed with `firstSegmentMs is not defined`. Update straight from 0.3.23 or earlier; do not stay on 0.3.24.

## 0.3.24

- **Fix**: Pulls proxy 2.9.106 — the browser's "time until playback" gets the two figures it needs from this host even on the first file opened after a restart, which is exactly the case it was built for. The stats line also says what is missing when it has nothing to report.

## 0.3.23

- **Fix**: Pulls proxy 2.9.105 — the download no longer stops dead while the encoder is held back. A reader's claim on the pieces it needs was being deleted by the torrent library as soon as those pieces arrived, and nothing put it back while the reader was parked; measured, that left a download at zero for eleven minutes with 150 peers connected.

## 0.3.22

- **Fix**: Pulls proxy 2.9.104 — a run whose input ran dry is no longer reported as a finished file, which is what left a viewer frozen 32 minutes in after the torrent's download died. A stalled download also reports itself now, with the figures that say whether the swarm was asked for anything at all.

## 0.3.21

- **Fix**: Pulls proxy 2.9.103 — playback worked in neither 2.9.101 nor 2.9.102. A constant used by both cold-start estimates was never declared, so every request to start a transcode session answered 500 and the browser reported the first segment missing. Update from 0.3.19 or 0.3.20 to play anything at all.

## 0.3.20

- **New**: Pulls proxy 2.9.102 — the playback plan also reports what this host takes to create a session, the second term of the browser's end-to-end estimate of time until playback.

## 0.3.19

- **New**: Pulls proxy 2.9.101 — the playback plan reports what this host takes to produce a session's first segment, which the browser needs to answer "how long until playback" during the gap where nothing else has a measured rate.

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
