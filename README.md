# Parallel Realms

A small cooperative C++17 / SFML 3.0.2 game. Two players explore separate
rooms, activate switches for each other, avoid moving hazards and escape.
An authoritative TCP server supports play across two computers.

## How to play

- Blue lives in the left room; red lives in the right room.
- Touch the yellow switch in your room. It stays activated and opens your
  partner's purple door. You do not need to remain on the switch.
- Cross the door and reach your green exit. Both players must finish to win.
- Touching an orange hazard sends both players back to their starting positions;
  activated switches remain on. A finished player is safe, but a hit to the other
  player resets both players' exit progress.
- R resets the whole shared round, including switches. Escape closes your client.

| Mode | Blue | Red |
| --- | --- | --- |
| Local, one keyboard | WASD | Arrow keys |
| LAN, separate computers | WASD or arrows | WASD or arrows |

The first client to connect is blue; the second is red. Both clients see both
rooms. In LAN mode the outlined player is yours. The game waits for two clients.
A disconnect clears that player's input; after detection the round resets and
waits for a replacement. Relaunch a disconnected client to join the vacant slot.

## Build on macOS (including Apple Silicon)

Install Xcode command-line tools, Git and CMake if missing:

```bash
xcode-select --install
brew install cmake
```

From this project's root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

CMake downloads pinned SFML 3.0.2 and its dependencies on the first build.
Internet is needed for that build; LAN gameplay itself does not need internet.
Build separately on each computer; do not copy a Mac executable to Windows.

## Two-computer setup

Both computers must have the same project source and a working build. Connect
both to the same normal Wi-Fi/router, or use Ethernet on the same LAN. They do
not need the same operating system. Allow incoming TCP **53000** on the host's
firewall. Client isolation on guest/college Wi-Fi can prevent connections.

### Computer A: host plus blue player

Terminal 1 (leave running):

```bash
./build/parallel_realms_server
```

Terminal 2:

```bash
./build/parallel_realms --connect 127.0.0.1
```

Find Computer A's local IPv4 address in its network settings. On macOS Wi-Fi,
`ipconfig getifaddr en0` often gives it; if blank, use the active network service
in System Settings instead. Share that local IP with Player 2.

### Computer B: red player

Replace the example IP below with Computer A's actual local IPv4 address:

```bash
./build/parallel_realms --connect 192.168.1.20
```

Do not use `127.0.0.1` on Computer B: it refers to Computer B itself.
When both clients connect, play starts. Stop the server with Ctrl+C.
If you choose a custom port, use it everywhere:

```bash
./build/parallel_realms_server 54000
./build/parallel_realms --connect 192.168.1.20 54000
```

For a one-Mac networking test, run the server and two clients, both connecting
to `127.0.0.1`. Only the focused window accepts keyboard input. This checks
networking locally, but the assignment demonstration should use two physical
computers.

### Local mode

```bash
./build/parallel_realms --local
```

No server or network is needed. Sounds, status panels and the victory overlay
are available in the network client.
For silent LAN play, append `--mute` to the client command.

## Linux and Windows

Ubuntu 24.04 dependencies:

```bash
sudo apt update
sudo apt install build-essential git cmake \
  libxrandr-dev libxcursor-dev libxi-dev libudev-dev libfreetype-dev \
  libflac-dev libvorbis-dev libgl1-mesa-dev libegl1-mesa-dev \
  libdrm-dev libgbm-dev
```

Then use the same build and launch commands as macOS.

Windows: install Git, CMake and Visual Studio 2022's **Desktop development with
C++** workload. From a developer terminal in the project root:

```powershell
cmake -S . -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build --config Release --parallel
.\build\Release\parallel_realms_server.exe
```

In a second terminal run
`.\build\Release\parallel_realms.exe --connect 127.0.0.1`.
The other computer uses the host's local IP. Permit the server on private
networks when Windows Firewall asks. Windows/macOS builds need local verification;
the supplied source uses SFML's cross-platform APIs.

## Features

- Separate server executable; clients send inputs, never trusted positions.
- Fixed 60 Hz server gameplay; latest snapshots distributed over TCP.
- Server main/simulation thread, network worker and hazard worker.
- Client main/render thread and network worker; SFML audio may use internal threads.
- Mutex-protected mailboxes, condition-variable wakeups, atomic stop flags and joined workers.
- Protocol versioning, endian-safe fixed-width integer positions, bounded receive work,
  pending-packet handling, input heartbeat and disconnect timeout.
- Sound cues for switches, hazards and winning; built-in font and synthesized sounds.
- Doors now span the passage: the partner's switch is necessary.
- Automated gameplay, serialization and real loopback TCP tests.

Tests are optional and disabled by default. See [manual checks](docs/TESTING.md)
for commands to enable and run them.

See [architecture](docs/ARCHITECTURE.md) and [manual checks](docs/TESTING.md).

## Limitations

This is a trusted-LAN classroom demo, not an internet matchmaking service.
There is no authentication, encryption, NAT traversal, prediction or interpolation.
Different internet networks need an additional routing/VPN solution; the simple
same-LAN setup is the intended demonstration. SFML packet receive trusts a length
prefix; do not expose this demo server to untrusted/public traffic. Reconnection
starts a fresh round rather than restoring a previous session. TCP can cause
visible delay on a congested network. Automated loopback tests cannot prove that
your Wi-Fi/firewall permits communication between two physical computers.
